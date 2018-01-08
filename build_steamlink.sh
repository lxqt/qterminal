#!/bin/bash
#

TOP=$(cd `dirname "${BASH_SOURCE[0]}"` && pwd)
if [ "${MARVELL_SDK_PATH}" = "" ]; then
	MARVELL_SDK_PATH="$(cd "${TOP}/../.." && pwd)"
fi
if [ "${MARVELL_ROOTFS}" = "" ]; then
	source "${MARVELL_SDK_PATH}/setenv.sh" || exit 1
fi
cd "${TOP}"

qmake
make $MAKE_J || exit 2

export DESTDIR="${PWD}/steamlink/apps/qterminal"

# Copy the files to the app directory
mkdir -p "${DESTDIR}"
cp -v qterminal "${DESTDIR}"

# Create the table of contents and icon
cat >"${DESTDIR}/toc.txt" <<__EOF__
name=qterminal
icon=icon.png
run=qterminal
__EOF__

base64 -d >"${DESTDIR}/icon.png" <<__EOF__
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
AABJRU5ErkJggg==
__EOF__

# Pack it up
name=$(basename ${DESTDIR})
pushd "$(dirname ${DESTDIR})"
tar zcvf $name.tgz $name || exit 3
rm -rf $name
popd

# All done!
echo "Build complete!"
echo
echo "Put the steamlink folder onto a USB drive, insert it into your Steam Link, and cycle the power to install."
