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
