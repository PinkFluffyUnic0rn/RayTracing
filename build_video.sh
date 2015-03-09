#!/bin/bash
avconv -r 60 -start_number 8 -i pngs/%d.png -b:v 1000000k test.mp4
