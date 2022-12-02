# Isopod

## Usage
```
git clone git@github.com:Zephyr-Queueing/Isopod.git
cd Isopod
sudo ./run.sh [server name] [thread number]
```
Allows user to connect worker to given existing Quartz server. As needed allows single host to scale-up multiple worker instances.

Note: Worker processes batches of json messages, the processing logic is easily modifiable.
