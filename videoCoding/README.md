# Video Coding

## encode examples
Using JPEG predictor 5 and intra frame mode 0  
`./codec encode videos/akiyo_qcif.y4m test 5 0`

#### encode using lossy compression  
`./codec encode videos/akiyo_qcif.y4m test 5 0 -lossy`

#### compute histograms and entropy  
`./codec encode videos/akiyo_qcif.y4m test 5 0 -hist`

## decode examples
`./codec decode test test.y4m`

## Extra mile
sudo apt install ffmpeg

### encode
`./extra encode ../videos/input5s.avi code_video code_audio`

### decode
`./extra decode code_vide code_audio new.avi`
