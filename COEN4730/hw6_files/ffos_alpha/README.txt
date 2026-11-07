1) 
first, you need an ALPHA cross-compiler, used inside Makefile:
/home/cristinel/hw4/gem5/cross_compilers/alphaev67-unknown-linux-gnu/bin/alphaev67-unknown-linux-gnu-cc 
create a new folder gem5/cross_compilers/
download the Pre-compiled Cross-compiler for ALPHA from: http://www.gem5.org/Download
get the "gcc-4.3.2, glibc-2.6.1 (NPTL,x86/64)" and place it gem5/cross_compilers/
then, unzip it like this:
> tar -vxjf alphaev67-unknown-linux-gnu.tar.bz2


2) 
to be able to run things, you first need to make changes to
(basically replace /home/cristinel/hw4/gem5 with your path):
--Makefile
--scr_cp2dm
--fs_modified.py

also, change your .bashrc file to include this line only at the end of it:
export M5_PATH=/home/cristinel/hw4/gem5/full_system_images_ALPHA
> emacs ~/.bashrc
> source ~/.bashrc

also, edit the file: gem5/configs/common/SysPaths.py 
And replace:
path = [ '/dist/m5/system', '/home/guoqi/simulators/gem5/dist/m5/system' ]
With:
path = [ '/dist/m5/system', '/home/cristinel/hw4/gem5/full_system_images_ALPHA' ]


3) 
read Makefile to get an idea about things;


4) 
to only compile application/benchmark:
> make all


5) 
to run one full simulation of the whole gem5 + mcpat flow:
> make run

Notes:
--fs_modified.py is a modified version of /home/cristinel/hw4/gem5/configs/example/fs.py
--m5op_alpha.S, m5op.h, m5ops.h ae just local copies of those from /home/cristinel/hw4/gem5/util/m5/
