set -x XMIPP_HOME /home/jordi/public/active/xmipp
set -x PATH $XMIPP_HOME/bin $PATH
set -x LD_LIBRARY_PATH $XMIPP_HOME/lib $LD_LIBRARY_PATH

# Xmipp Aliases
## Setup ##
alias xconfigure './setup.py -j 1 configure compile '
alias xcompile './setup.py -j 1 compile '
alias xupdate './setup.py -j 1 update compile '
## Interface ##
alias xa 'xmipp_apropos'
alias xb 'xmipp_browser'
alias xp 'xmipp_protocols'
alias xmipp 'xmipp_protocols'
alias xs 'xmipp_showj'
alias xmipp_show 'xmipp_showj'
alias xsj 'xmipp_showj'
alias xij 'xmipp_imagej'
## Image ##
alias xic 'xmipp_image_convert'
alias xih 'xmipp_image_header'
alias xio 'xmipp_image_operate'
alias xis 'xmipp_image_statistics'
## Metadata ##
alias xmu 'xmipp_metadata_utilities'
alias xmp 'xmipp_metadata_plot'
## Transformation ##
alias xtg 'xmipp_transform_geometry'
alias xtf 'xmipp_transform_filter'
alias xtn 'xmipp_transform_normalize'
## Other ##
alias xrf 'xmipp_resolution_fsc'
alias xrs 'xmipp_resolution_ssnr'


## Configuration ##

# This file will serve to customize some settings of you Xmipp installation
# Each setting will be in the form o a shell variable set to some value

#---------- GUI ----------
# If you set to 1 the value of this variable, by default the programs
# will launch the gui when call without argments, default is print the help
set -x XMIPP_GUI_DEFAULT 0

# If you set to 0 the value of this variable the script generated
# by programs gui will not be erased and you can use the same parameters
set -x XMIPP_GUI_CLEAN 1

#---------- Parallel ----------
# This variable will point to your job submition template file
set -x XMIPP_PARALLEL_LAUNCH config_launch.py

#---------- Font ----------
# These variables set your personal font configuration
set -x XMIPP_FONT_NAME Verdana
set -x XMIPP_FONT_SIZE 10

# If you have .xmipp.cfg in your home folder it will override
# this configurations

if test -s ~/.xmipp.cfg
  source ~/.xmipp.cfg
else
  true
end
