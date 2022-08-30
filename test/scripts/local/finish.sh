#!/bin/bash
ps -aef | grep unifyfsd | grep -v grep | awk '{print $2}' | xargs kill -9 > /dev/null 2>&1