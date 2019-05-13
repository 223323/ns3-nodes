## clone repository
```
cd ns-3.29/examples
git clone https://github.com/223323/ns3-nodes.git
cd ns3-nodes
cp run ../../
```

## running
```
cd ns-3.29
./run rtrk-nodes
```

## results
```
cd ns-3.29/work/rtrk-nodes/
```

- channel_stats.csv:
Each row represents channels current usage in Bytes per second.
Each row is for each second passed.
- ips.txt:
Shows reaper ip addresses and their channel index.
Same index means they are directly connected.
