#!/bin/sh

# set -x

# Package VERSIONs. Change these when releasing new packages
TCLVER=8.5.10
TKVER=8.5.10
OTCLVER=1.14
TCLCLVER=1.20
NSVER=2.35
TCLDEBUGVER=2.0

# Get current path
CUR_PATH=`pwd`

echo "============================================================"
echo "* Build tcl-debug-$TCLDEBUGVER"
echo "============================================================"

cd ./tcl-debug-$TCLDEBUGVER
if [ -f Makefile ] ; then 
	make distclean
fi

# TODO test_cygwin
# ./configure --with-otcl=../otcl-$OTCLVER --with-tclcl=../tclcl-$TCLCLVER --with-tcl-ver=$TCLVER --with-tk-ver=$TKVER || die "Ns configuration failed! Exiting ...";
./configure --with-tcl="${CUR_PATH}/lib" --with-tclinclude="$CUR_PATH/tcl$TCLVER/generic" --prefix=${CUR_PATH} --enable-gcc || die "Ns configuration failed! Exiting ..."

if make
then
	echo "tcl-debug-$TCLDEBUGVER make succeeded."
	make install || die "tcl$TCLVER installation failed."
	echo "tcl-debug-$TCLDEBUGVER installation succeeded."
else
	echo "tcl-debug-$TCLDEBUGVER make failed! Exiting ..."
	exit
fi

cd ..

echo "============================================================"
echo "* Build ns-$NSVER"
echo "============================================================"

cd ./ns-$NSVER
if [ -f Makefile ] ; then 
	make distclean
fi

# if  [ "${test_cygwin}" = "true" ]; then
#         ./configure --x-libraries=/usr/X11R6/lib --x-includes=/usr/X11R6/include --with-tcl-ver=$TCLVER --with-tk-ver=$TKVER || die "Ns configuration failed! Exiting ...";
# else
        ./configure --with-otcl=../otcl-$OTCLVER --with-tclcl=../tclcl-$TCLCLVER --with-tcl-ver=$TCLVER --with-tk-ver=$TKVER  --with-tcldebug || die "Ns configuration failed! Exiting ...";
# fi

if make
then
	echo " Ns has been installed successfully." 
else
	echo "Ns make failed!"
	echo "See http://www.isi.edu/nsnam/ns/ns-problems.html for problems"
	exit
fi

cd ../

# Install nam, ns, xgraph into bin

if [ ! -d bin ] ; then
    mkdir bin
fi

cd bin

ln -s $CUR_PATH/ns-$NSVER/ns${EXE} ns${EXE}

if test -x $CUR_PATH/nam-$NAMVER/nam${EXE}
then
    ln -s $CUR_PATH/nam-$NAMVER/nam${EXE} nam${EXE}
else
    echo "Please compile your nam separately."
fi

if test -x $CUR_PATH/xgraph-$XGRAPHVER/xgraph${EXE}
then
    ln -s $CUR_PATH/xgraph-$XGRAPHVER/xgraph${EXE} xgraph${EXE}
else
    echo "Please compile your xgraph separately."
fi

if test -x $CUR_PATH/gt-itm/bin/sgb2ns${EXE}
then 
    ln -s $CUR_PATH/gt-itm/bin/sgb2ns${EXE} sgb2ns${EXE}
    ln -s $CUR_PATH/gt-itm/bin/sgb2hierns${EXE} sgb2hierns${EXE}
    ln -s $CUR_PATH/gt-itm/bin/sgb2comns${EXE} sgb2comns${EXE}
    ln -s $CUR_PATH/gt-itm/bin/itm${EXE} itm${EXE}
    ln -s $CUR_PATH/gt-itm/bin/sgb2alt${EXE} sgb2alt${EXE}
    ln -s $CUR_PATH/gt-itm/bin/edriver${EXE} edriver${EXE}
else
    echo "Please compile your gt-itm & sgb2ns separately."
fi

echo ""
echo "Ns-allinone package has been installed successfully."
echo "Here are the installation places:"
echo "tcl$TCLVER:	$CUR_PATH/{bin,include,lib}"
echo "tk$TKVER:		$CUR_PATH/{bin,include,lib}"
echo "otcl:		$CUR_PATH/otcl-$OTCLVER"
echo "tclcl:		$CUR_PATH/tclcl-$TCLCLVER"
echo "ns:		$CUR_PATH/ns-$NSVER/ns"

if [ -x $CUR_PATH/nam-$NAMVER/nam ]
then
echo "nam:	$CUR_PATH/nam-$NAMVER/nam"
fi

if [ -x $CUR_PATH/xgraph-$XGRAPHVER/xgraph ]
then
echo "xgraph:	$CUR_PATH/xgraph-$XGRAPHVER"
fi
if [ -x $CUR_PATH/gt-itm/bin/sgb2ns ] 
then
echo "gt-itm:   $CUR_PATH/itm, edriver, sgb2alt, sgb2ns, sgb2comns, sgb2hierns"
fi

echo ""
echo "----------------------------------------------------------------------------------"
echo ""
echo "Please put $CUR_PATH/bin:$CUR_PATH/tcl$TCLVER/unix:$CUR_PATH/tk$TKVER/unix" 
echo "into your PATH environment; so that you'll be able to run itm/tclsh/wish/xgraph."
echo ""
echo "IMPORTANT NOTICES:"
echo ""
echo "(1) You MUST put $CUR_PATH/otcl-$OTCLVER, $CUR_PATH/lib, "
echo "    into your LD_LIBRARY_PATH environment variable."
echo "    If it complains about X libraries, add path to your X libraries "
echo "    into LD_LIBRARY_PATH."
echo "    If you are using csh, you can set it like:"
echo "		setenv LD_LIBRARY_PATH <paths>"
echo "    If you are using sh, you can set it like:"
echo "		export LD_LIBRARY_PATH=<paths>"
echo ""
echo "(2) You MUST put $CUR_PATH/tcl$TCLVER/library into your TCL_LIBRARY environmental"
echo "    variable. Otherwise ns/nam will complain during startup."
echo ""
echo ""
echo "After these steps, you can now run the ns validation suite with"
echo "cd ns-$NSVER; ./validate"
echo ""
echo "For trouble shooting, please first read ns problems page "
echo "http://www.isi.edu/nsnam/ns/ns-problems.html. Also search the ns mailing list archive"
echo "for related posts." 
echo ""

exit 0
