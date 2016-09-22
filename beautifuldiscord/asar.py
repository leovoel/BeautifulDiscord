import io
import os
import json
import struct
import shutil


def align_int(i, alignment):
    """Rounds up `i` to the next multiple of `alignment`."""
    return i + (alignment - (i % alignment)) % alignment


class Asar:

    """Represents an asar file.

    You probably want to use the ``Asar.open`` or ``Asar.from_path``
    class methods instead of creating an instance of this class.

    Attributes
    ----------
    path: str
        Path of this asar file on disk.
        If ``Asar.from_path`` is used, this is just
        the path given to it.
    fp: File-like object
        Contains the data for this asar file.
    header: dict
        Dictionary used for random file access.
    base_offset: int
        Indicates where the asar file header ends.
    """

    def __init__(self, path, fp, header, base_offset):
        self.path = path
        self.fp = fp
        self.header = header
        self.base_offset = base_offset

    @classmethod
    def open(cls, path):
        """Decodes the asar file from the given ``path``.

        You should probably use the context manager interface here,
        to automatically close the file object when you're done with it, i.e.

        .. code-block:: python

            with Asar.open('./something.asar') as a:
                a.extract('./something_dir')

        Parameters
        ----------
        path: str
            Path of the file to be decoded.
        """
        fp = open(path, 'rb')

        # decode header
        # NOTE: we only really care about the last value here.
        data_size, header_size, header_object_size, header_string_size = struct.unpack('<4I', fp.read(16))

        header_json = fp.read(header_string_size).decode('utf-8')

        return cls(
            path=path,
            fp=fp,
            header=json.loads(header_json),
            base_offset=align_int(16 + header_string_size, 4)
        )

    @classmethod
    def from_path(cls, path):
        """Creates an asar file using the given ``path``.
        
        When this is used, the ``fp`` attribute of the returned instance
        will be a ``io.BytesIO`` object, so it's not written to a file.
        You have to do something like:

        .. code-block:: python
            
            with Asar.from_path('./something_dir') as a:
                with open('./something.asar', 'wb') as f:
                    a.fp.seek(0) # just making sure we're at the start of the file
                    f.write(a.fp.read())

        Parameters
        ----------
        path: str
            Path to walk into, recursively, and pack
            into an asar file.
        """
        offset = 0
        concatenated_files = b''

        def _path_to_dict(path):
            nonlocal concatenated_files, offset
            result = {'files': {}}

            for f in os.scandir(path):
                if os.path.isdir(f.path):
                    result['files'][f.name] = _path_to_dict(f.path)
                else:
                    size = f.stat().st_size

                    result['files'][f.name] = {
                        'size': size,
                        'offset': str(offset)
                    }

                    with open(f.path, 'rb') as fp:
                        concatenated_files += fp.read()

                    offset += size

            return result

        header = _folder_to_dict(path)
        header_json = json.dumps(header, sort_keys=True, separators=(',', ':')).encode('utf-8')

        # TODO: using known constants here for now (laziness)...
        #       we likely need to calc these, but as far as discord goes we haven't needed it.
        header_string_size = len(header_json)
        data_size = 4 # uint32 size
        aligned_size = align_int(header_string_size, data_size)
        header_size = aligned_size + 8
        header_object_size = aligned_size + data_size

        # pad remaining space with NULLs
        diff = aligned_size - header_string_size
        header_json = header_json + b'\0' * (diff) if diff else header_json

        fp = io.BytesIO()
        fp.write(struct.pack('<4I', data_size, header_size, header_object_size, header_string_size))
        fp.write(header_json)
        fp.write(concatenated_files)

        return cls(
            path=path,
            fp=fp,
            header=header,
            base_offset=align_int(16 + header_string_size, 4)
        )

    def _copy_extracted(self, source, destination):
        unpacked_dir = self.filename + '.unpacked'
        if not os.path.isdir(unpacked_dir):
            print("Couldn't copy file {}, no extracted directory".format(source))
            return

        src = os.path.join(unpacked_dir, source)
        if not os.path.exists(src):
            print("Couldn't copy file {}, doesn't exist".format(src))
            return

        dest = os.path.join(destination, source)
        shutil.copyfile(src, dest)

    def _extract_file(self, source, info, destination):
        if 'offset' not in info:
            self._copy_extracted(source, destination)
            return

        self.fp.seek(self.base_offset + int(info['offset']))
        r = self.fp.read(int(info['size']))

        dest = os.path.join(destination, source)
        with open(dest, 'wb') as f:
            f.write(r)

    def _extract_directory(self, source, files, destination):
        dest = os.path.normcase(os.path.join(destination, source))

        if not os.path.exists(dest):
            os.makedirs(dest)

        for name, info in files.items():
            item_path = os.path.join(source, name)

            if 'files' in info:
                self._extract_directory(item_path, info['files'], destination)
                continue

            self._extract_file(item_path, info, destination)

    def extract(self, path):
        """Extracts this asar file to ``path``.
    
        Parameters
        ----------
        path: str
            Destination of extracted asar file.
        """
        if os.path.exists(path):
            raise FileExistsError()

        self._extract_directory('.', self.header['files'], path)

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.fp.close()
