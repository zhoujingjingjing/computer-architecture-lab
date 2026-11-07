1) 
to be able to run things, you first need to make changes to
(basically replace /home/cristinel/hw4/gem5 with your path):
--Makefile
--scr_cp2dm
--fs_modified.py

also, change your .bashrc file to include this line only at the end of it:
export M5_PATH=/home/cristinel/hw4/gem5/full_system_images_ARM
> emacs ~/.bashrc
> source ~/.bashrc

also, edit the file: gem5/configs/common/SysPaths.py 
And replace:
path = [ '/dist/m5/system', '/home/guoqi/simulators/gem5/dist/m5/system' ]
With:
path = [ '/dist/m5/system', '/home/cristinel/hw4/gem5/full_system_images_ARM' ]


2)
read Makefile to get an idea about the steps that are done;


3)
to only compile application/benchmark:
> make all


4)
to run one full simulation of the whole gem5 + mcpat flow:
> make run


Notes:
--fs_modiified.py is a modified version of /home/cristinel/hw4/gem5/configs/example/fs.py
--m5op_arm.S, m5op.h, m5ops.h are just local copies of those from /home/cristinel/hw4/gem5/util/m5/
