# LICENSE: CC0
#
# "real time" plotting of gnuplot.
#
# Assumes a voxel gnuplot like data file, `viz.gp`,
# exists that it will plot:
#
# ...
# x y z v
# ...
#
# To not have the plot dynamically resize, both in
# spatial dimension and in palette, the output `vis.gp`
# should include two points at either end of the bounding
# box with the extremes of the palette value (that is, 0 and 1).
#
# pointtype 5       : filled in square
# pointsize 0.75    : big enough?
# linecolor palette : default palette
#
# pause 0.25        : pause for 0.25 seconds
#
# Explicitely setting ranges and view will reset those values
# on reread, so best let the program that's outputting the `viz.gp`
# figure it out.
#
# Note: the poms command line program creates a temporary file that
# is then moved into `viz.gp` to try and avoid gnuplot doing a partial
# read of the file as system the move command should be atomic.
#

#set xrange [0:1]
#set yrange [0:1]
#set zrange [0:1]
#set view 64,40
#set view 0,10

set zlabel "Z"
set ylabel "Y"
set xlabel "X"
show xlabel
show ylabel
show zlabel

splot 'viz.gp' using 1:2:3:4 with points pointtype 5 pointsize 0.75 linecolor palette
pause 0.25
reread
