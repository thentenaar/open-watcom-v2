# wccmps Builder Control file
# ===========================

set PROJNAME=wccmps

set BINTOOL=0

set PROJDIR=<CWD>

[ INCLUDE <OWROOT>/build/prolog.ctl ]

[ INCLUDE <OWROOT>/build/defrule.ctl ]

[ BLOCK <BLDRULE> rel ]
#======================
    cdsay <PROJDIR>

[ BLOCK <BINTOOL> build ]
#========================
    cdsay <PROJDIR>
    <CPCMD> <OWOBJDIR>/bwccmps.exe      <OWBINDIR>/<OWOBJDIR>/bwccmps<CMDEXT>
    <CCCMD> <OWOBJDIR>/bwccdmps<DYEXT>  <OWBINDIR>/<OWOBJDIR>/bwccdmps<DYEXT>

[ BLOCK <BINTOOL> clean ]
#========================
    echo rm -f <OWBINDIR>/<OWOBJDIR>/bwccmps<CMDEXT>
    rm -f <OWBINDIR>/<OWOBJDIR>/bwccmps<CMDEXT>
    rm -f <OWBINDIR>/<OWOBJDIR>/bwccdmps<DYEXT>

[ BLOCK <BLDRULE> rel cprel ]
#============================
    <CCCMD> dos386/wccmps.exe        <OWRELROOT>/binw/
    <CCCMD> dos386/wccmps.sym        <OWRELROOT>/binw/
    <CCCMD> dos386/wccmps01.int      <OWRELROOT>/binw/
    <CCCMD> os2386.dll/wccmps.exe    <OWRELROOT>/binp/
    <CCCMD> os2386.dll/wccmps.sym    <OWRELROOT>/binp/
    <CCCMD> os2386.dll/wccdmps.dll   <OWRELROOT>/binp/dll/
    <CCCMD> os2386.dll/wccdmps.sym   <OWRELROOT>/binp/dll/
    <CCCMD> os2386.dll/wccmps01.int  <OWRELROOT>/binp/dll/
    <CCCMD> nt386.dll/wccmps.exe     <OWRELROOT>/binnt/
    <CCCMD> nt386.dll/wccmps.sym     <OWRELROOT>/binnt/
    <CCCMD> nt386.dll/wccdmps.dll    <OWRELROOT>/binnt/
    <CCCMD> nt386.dll/wccdmps.sym    <OWRELROOT>/binnt/
    <CCCMD> nt386.dll/wccmps01.int   <OWRELROOT>/binnt/
    <CCCMD> ntaxp.dll/wccmps.exe     <OWRELROOT>/axpnt/
    <CCCMD> ntaxp.dll/wccmps.sym     <OWRELROOT>/axpnt/
    <CCCMD> ntaxp.dll/wccdmps.dll    <OWRELROOT>/axpnt/
    <CCCMD> ntaxp.dll/wccdmps.sym    <OWRELROOT>/axpnt/
    <CCCMD> ntaxp.dll/wccmps01.int   <OWRELROOT>/axpnt/
    <CCCMD> qnx386/wccmps.exe        <OWRELROOT>/qnx/binq/wccmps
    <CCCMD> qnx386/wccmps.sym        <OWRELROOT>/qnx/sym/
    <CCCMD> qnx386/wccmps01.int      <OWRELROOT>/qnx/binq/
    <CCCMD> linux386/wccmps.exe      <OWRELROOT>/binl/wccmps
    <CCCMD> linux386/wccmps.sym      <OWRELROOT>/binl/
    <CCCMD> linux386/wccmps01.int    <OWRELROOT>/binl/

    <CCCMD> ntx64.dll/wccmps.exe     <OWRELROOT>/binnt64/
    <CCCMD> ntx64.dll/wccdmps.dll    <OWRELROOT>/binnt64/
    <CCCMD> ntx64.dll/wccmps01.int   <OWRELROOT>/binnt64/
    <CCCMD> linuxx64/wccmps.exe      <OWRELROOT>/binl64/wccmps
    <CCCMD> linuxx64/wccmps01.int    <OWRELROOT>/binl64/
    <CCCMD> linuxarm/wccmps.exe      <OWRELROOT>/arml/wccmps
    <CCCMD> linuxarm/wccmps01.int    <OWRELROOT>/arml/
    <CCCMD> osxx64/wccmps.exe        <OWRELROOT>/osx64/wccmps
    <CCCMD> osxx64/wccmps01.int      <OWRELROOT>/osx64/

[ BLOCK . . ]

[ INCLUDE <OWROOT>/build/epilog.ctl ]
