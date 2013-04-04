export SOAROOT=~/git/soa

if [ -z "${CPATH}" ] ; then
	CPATH="${SOAROOT}/include"; export CPATH
else
	CPATH="${SOAROOT}/include:$CPATH"; export CPATH
fi
