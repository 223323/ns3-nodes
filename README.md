
## prepare environment
cd ns-3.29
touch run
chmod +x run

copy content to run file
```
#!/bin/bash
cmd="${1:+$@}"
cwd_dir=work/$1
mkdir -p work
rm -rf $cwd_dir/*
mkdir -p $cwd_dir
echo ./waf --cwd $cwd_dir --run $1
./waf --cwd $cwd_dir --run $1
```


## running
cd ns-3.29
./run rtrk-nodes


## results
cd ns-3.29/work/rtrk-nodes/

- channel_stats.csv
Each row represents channels current usage in Bytes per second.
Each row is for each second passed.
- ips.txt
Shows reaper ip addresses and their channel index.
Same index means they are directly connected.
