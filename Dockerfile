FROM archlinux/base:latest

RUN \
    pacman -Syu \
    base-devel \
    git \
    python \
    boost-libs \
    --noconfirm

RUN useradd -ms /bin/bash placer
USER placer
WORKDIR /home/placer
