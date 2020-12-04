# Audio and Image/Video Manipulation


### Install dependencies.
```
pip install -r requirements.txt         OR
pip install --user -r requirements.txt  OR
pip3 install -r requirements.txt        OR
pip3 install --user -r requirements.txt 
```

### Build the entire project.
```
mkdir build && cd build
cmake ..
cmake --build .
cd ..
```

### Run the scripts

#### Copy a wav file, sample by sample. 
``` 
./build/copywav ../wav_files/sample01.wav ./src/audio/copiedwav.wav
```

#### Copy a video, pixel by pixel.
```

```

#### Display a video on the screen.
```

```

#### Histograms of an audio sample (left and rigth channel and mono) and corresponding entropy.
```
./build/audioEnt ../wav_files/sample01.wav
python3 ./src/audio/plotHist.py
```

#### Histogram of an image/video file (each channel in color and grayscale images) and the corresponding entropy.
```

```

#### Reduce the number of bits used to represent each audio sample (uniform scalar quantization).
```

```

#### Reduce the number of bits used to represent each pixel of an image/video.
```

```

#### Print the SNR of the relation between two audio files, and the maximum per sample absolute error.
```

```

#### Print the SNR of the relation between two image/video files, and the maximum per pixel absolute error.
```

```
