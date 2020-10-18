# Image-Processing-and-Data-compression
Project of the Audio And Video Coding course, University of Aveiro

## Installation of libsndfile
Install libsndfile by running the following commands:
```
./configure --prefix=/usr    \
            --disable-static \
            --docdir=/usr/share/doc/libsndfile-1.0.28 &&
make
```
To test the results, issue: make check.
        
Now, as the root user:
```
make install
```