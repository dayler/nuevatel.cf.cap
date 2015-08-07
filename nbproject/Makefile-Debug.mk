#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=cc
CCC=CC
CXX=CC
FC=f95
AS=as

# Macros
CND_PLATFORM=OracleSolarisStudio-Solaris-x86
CND_DLIB_EXT=so
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/main.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=-mt -m64 -library=stlport4,no%Cstd -R/opt/ulcm/library -L/usr/local/lib -L/opt/ulcm/library -pthreads -lboost_thread -lresolv -lnsl -lFt -ldf -lapi -lscos -lsccp -lsocket -lrt -ldgms -lm -lswrmm -loos -ltcap -lcapv480
CXXFLAGS=-mt -m64 -library=stlport4,no%Cstd -R/opt/ulcm/library -L/usr/local/lib -L/opt/ulcm/library -pthreads -lboost_thread -lresolv -lnsl -lFt -ldf -lapi -lscos -lsccp -lsocket -lrt -ldgms -lm -lswrmm -loos -ltcap -lcapv480

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/trunk

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/trunk: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/trunk ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/main.o: main.cpp 
	${MKDIR} -p ${OBJECTDIR}
	$(COMPILE.cc) -g +w -DC7_Q -D_REENTRANT -I/usr/local/include -I/opt/ulcm/include -o ${OBJECTDIR}/main.o main.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/trunk
	${CCADMIN} -clean

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
