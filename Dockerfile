FROM ubuntu:18.04.02

ENV TERM screen-256color
ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update -y \
	 sudo add-apt-repository ppa:alexlarsson/flatpak -y \
	 sudo apt update -y \
	 sudo apt install git flatpak python3-dev python3-pip gnome-software-plugin-flatpak -y \
	 flatpak remote-add --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo -y \
	 flatpak install flathub com.discordapp.Discord -y \
	 git clone https://github.com/leovoel/BeautifulDiscord.git
