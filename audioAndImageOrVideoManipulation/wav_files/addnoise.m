% Open standard demo sound that ships with MATLAB.
[perfectSound, freq] = audioread('sample01.wav');
% Add noise to it.
noisySound = perfectSound + 0.2 * randn(length(perfectSound), 1);
% write 
audiowrite('sample01_noise.wav', noisySound, freq);
