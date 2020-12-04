# Image-Processing-and-Data-compression
Project of the Audio And Video Coding course, University of Aveiro

## Project
This project was developed following milestones, each one with their corresponding folder:
 
 1) [**Audio And Image/Video Manipulation**](./audioAndImageOrVideoManipulation)
 2) [**Entropy Coding**](./entropyCoding)
 3) [**Audio Coding**](./audioCoding)
 4) [**Video Coding**](./videoCoding)
 5) [**Extra Mile**](./extraMile)

## Documentation
```
cd docs && doxygen && firefox html/index.html
```

## Installation of libsndfile
Decrompress the file, on the cmd go to the repository of the file libsndfile-1.0.28 
LinuxCMD:~.../libsndfile-1.0.28

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
