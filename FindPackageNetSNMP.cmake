# Find net-snmp opensourse package include & library path 
#
FIND_PATH(NET-SNMP_INCLUDE 
	snmp.h 
	/usr/local/include/net-snmp/library)
	
FIND_LIBRARY(NET-SNMP_LIBRARY 
	NAMES netsnmp 
	PATHS /usr/lib /usr/local/lib )

INCLUDE(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(NET-SNMP DEFAULT_MSG 
	NET-SNMP_INCLUDE 
	NET-SNMP_LIBRARY)

MARK_AS_ADVANCED(NET-SNMP_LIBRARY NET-SNMP_INCLUDE)
