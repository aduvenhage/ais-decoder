FROM python:3.6-stretch
MAINTAINER Rory <rmeyer.csir@gmail.com>
RUN apt-get update
RUN apt-get install -y python3-dev swig g++ cmake make
RUN python3 -m ensurepip
RUN mkdir /usr/local/code && mkdir /usr/local/data
COPY ais_testing/requirements.txt /usr/local/code
RUN pip3 install -r /usr/local/code/requirements.txt
RUN git clone https://github.com/aduvenhage/ais-decoder.git && cd ais-decoder && git checkout master 
RUN mkdir ais-decoder-build && cd ais-decoder-build && cmake ../ais-decoder -DCMAKE_BUILD_TYPE=RELEASE && make && make install
RUN cd ais-decoder/python && python setup.py build && python setup.py install
WORKDIR /usr/local/code