#!/bin/sh

make -B
sudo ./launcher taskset.txt
cat /proc/moker_trace > trace.csv
cat trace.csv | grep -E "(ENQ_RQ|DEQ_RQ|SWT_TO|SWT_AY|ENQ_WQ|DEQ_WQ|MUT_LK|MUT_UL|MUT_UP|PRIO_CHG)" | grep ",task$" > trace_f.csv