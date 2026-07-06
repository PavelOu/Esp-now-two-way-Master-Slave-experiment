# Esp-now-two-way-Master-Slave-experiment
Due of measuring of ECG and BPM I’ve started to tests  possibilities of ESP-NOW Master and Slaves communication systém with ESP modules

Esp-now-two-way Master + Slave Q experiment						6.7.2026

	I've improved my Esp-now M-S experiment o  Q variante, so I think it work more quick as previous one .
At first I've got off osc sytem (commands M+N) and I've shorted the dstruct messane on one bzte wit command and one unsigned int for data. There are examples of sketch for master and first slave . Every slave has your slaveNumber and if you will test this system, you must use at first command Kx, where x is slave nummer.
