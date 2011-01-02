#!/usr/bin/env python

import datetime
import sys

def log(msg):
    now = datetime.datetime.now()
    (year, mon, day, hour, minute, second, weekday, days, dst) = now.timetuple()
    sys.stderr.write("%04d.%02d.%02d %02d:%02d:%02d %s\n" % \
                    (year, mon, day, hour, minute, second, msg))
    sys.stderr.flush()
