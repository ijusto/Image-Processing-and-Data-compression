# Audio Coding

### Build the entire project.
```
mkdir build && cd build
cmake ..
cmake --build .
cd ..
```

## encode examples
`./build/audiocodec encode ../wav_files/sample01.wav test`

#### encode using lossy compression  
`./build/audiocodec encode ../wav_files/sample01.wav test -lossy`

#### compute histograms and entropy  
`./build/audiocodec encode ../wav_files/sample01.wav test -hist`

## decode examples
`./build/audiocodec decode test test.wav`
