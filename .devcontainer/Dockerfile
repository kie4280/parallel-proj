# FROM ubuntu:focal

# RUN apt-get -yy remove texlive*
# ENV DEBIAN_FRONTEND=noninteractive
# RUN apt-get update && apt-get -yy upgrade

# RUN apt-get -yy install texlive-xetex texlive-extra-utils latexmk




FROM texlive/texlive:latest

RUN tlmgr update --self && \
    tlmgr update --all

RUN tlmgr install xetex

RUN tlmgr install libertine inconsolata && \
    tlmgr install mathexam setspace adjustbox xkeyval collectbox enumitem lastpage && \
    texhash

RUN apt-get -yy install git
