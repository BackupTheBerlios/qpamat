# (c) 2008-2009, Bernhard Walle <bernhard@bwalle.de>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#

PROJECT(qpamat)
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

INCLUDE(FindOpenSSL)
INCLUDE(FindQt4)
INCLUDE(CTest)
INCLUDE(FindX11)
SET(QT_USE_QT3SUPPORT 1)
SET(QT_USE_QTXML      1)
SET(QT_USE_QTMAIN     1) # only Windows
SET(QT_USE_QTTEST     1)
IF (CMAKE_HOST_UNIX AND NOT CMAKE_HOST_APPLE)
    SET(QT_USE_QTDBUS     1)
ENDIF (CMAKE_HOST_UNIX AND NOT CMAKE_HOST_APPLE)
INCLUDE(${QT_USE_FILE})

# include the current working directory
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src)
INCLUDE_DIRECTORIES(${OPENSSL_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/src)

# version
file(STRINGS "VERSION" VERSION_STRING)
ADD_DEFINITIONS(-DVERSION_STRING="${VERSION_STRING}")

# always print compiler warnings
IF (CMAKE_HOST_UNIX)
    ADD_DEFINITIONS(-Wall -W -Wno-unused-parameter)
ENDIF (CMAKE_HOST_UNIX)

IF (MSVC)
    # MSVC++ ignores all exception specifications except '()' [i.e. no exception]
    # we can safely ignore that warning when compiling on MS Windows
    ADD_DEFINITIONS("/w44290")

    # we also don't want useless warnings that we should use some Windows specific functions
    # instead of Standard C functions (which is quite right because the standard C functions
    # are not safe, but we have no alternative in portable code)
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS=1)

    # also get rid of warnings "Function call with parameters that may be unsafe"
    # the code is correct C++!
    ADD_DEFINITIONS(-D_SCL_SECURE_NO_WARNINGS=1)
ENDIF (MSVC)


SET(qpamat_SRCS
    src/ext/getopt.cpp
    src/dialogs/passworddialog.cpp
    src/dialogs/newpassworddialog.cpp
    src/dialogs/configurationdialog.cpp
    src/dialogs/showpassworddialog.cpp
    src/dialogs/waitdialog.cpp
    src/dialogs/insertcarddialog.cpp
    src/dialogs/cardpinvalidator.cpp
    src/dialogs/aboutdialog.cpp
    src/widgets/filelineedit.cpp
    src/widgets/fontchoosebox.cpp
    src/widgets/copylabel.cpp
    src/widgets/focuslineedit.cpp
    src/widgets/listboxlabeledpict.cpp
    src/widgets/listboxdialog.cpp
    src/security/encodinghelper.cpp
    src/security/passwordhash.cpp
    src/security/abstractencryptor.cpp
    src/security/symmetricencryptor.cpp
    src/security/collectencryptor.cpp
    src/security/randompasswordgenerator.cpp
    src/security/externalpasswordgenerator.cpp
    src/security/passwordgeneratorfactory.cpp
    src/security/passwordchecker.cpp
    src/security/hybridpasswordchecker.cpp
    src/security/masterpasswordchecker.cpp
    src/smartcard/cardexception.cpp
    src/smartcard/memorycard.cpp
    src/smartcard/nosuchlibraryexception.cpp
    src/smartcard/notinitializedexception.cpp
    src/util/stringdisplay.cpp
    src/util/singleapplication.cpp
    src/util/timeoutapplication.cpp
    src/util/securestring.cpp
    src/util/ansicolor.cpp
    src/util/debug.cpp
    src/util/msghandler.cpp
    src/datareadwriter.cpp
    src/timerstatusmessage.cpp
    src/randompassword.cpp
    src/treeentry.cpp
    src/property.cpp
    src/tree.cpp
    src/settings.cpp
    src/qpamatwindow.cpp
    src/qpamat.cpp
    src/help.cpp
    src/rightpanel.cpp
    src/rightlistview.cpp
    src/southpanel.cpp
    src/main.cpp
)

SET(qpamat_MOCS
    src/dialogs/passworddialog.h
    src/dialogs/configurationdialog.h
    src/dialogs/insertcarddialog.h
    src/dialogs/newpassworddialog.h
    src/dialogs/aboutdialog.h
    src/dialogs/showpassworddialog.h
    src/dialogs/configurationdialogprivate.h
    src/util/singleapplication.h
    src/util/timeoutapplication.h
    src/widgets/listboxdialog.h
    src/widgets/filelineedit.h
    src/widgets/fontchoosebox.h
    src/widgets/focuslineedit.h
    src/widgets/copylabel.h
    src/randompassword.h
    src/southpanel.h
    src/timerstatusmessage.h
    src/rightlistview.h
    src/tree.h
    src/rightpanel.h
    src/property.h
    src/treeentry.h
    src/help.h
    src/qpamatwindow.h
)

# build some files only on specific platforms
IF (CMAKE_HOST_UNIX)
    SET(qpamat_SRCS
        ${qpamat_SRCS}
        src/util/processinfo_unix.cpp
        src/util/platformhelpers_posix.cpp
    )
    IF (NOT CMAKE_HOST_APPLE)
        SET(qpamat_SRCS ${qpamat_SRCS} src/qpamatadaptor.cpp)
        SET(qpamat_MOCS ${qpamat_MOCS} src/qpamatadaptor.h)
    ENDIF (NOT CMAKE_HOST_APPLE)
ENDIF (CMAKE_HOST_UNIX)

IF (CMAKE_HOST_WIN32)
    SET(qpamat_SRCS
        ${qpamat_SRCS}
        src/util/processinfo_win.cpp
        src/util/platformhelpers_win32.cpp
        share/win32/qpamat_win32.rc
    )
    # copy icons
    CONFIGURE_FILE(
       ${CMAKE_SOURCE_DIR}/share/win32/qpamat_34.ico
       ${CMAKE_BINARY_DIR}/qpamat_34.ico
       COPYONLY
    )
    CONFIGURE_FILE(
       ${CMAKE_SOURCE_DIR}/share/win32/qpamat_16.ico
       ${CMAKE_BINARY_DIR}/qpamat_16.ico
       COPYONLY
    )
    CONFIGURE_FILE(
       ${CMAKE_SOURCE_DIR}/share/win32/qpamat_48.ico
       ${CMAKE_BINARY_DIR}/qpamat_48.ico
       COPYONLY
    )
ENDIF (CMAKE_HOST_WIN32)

# compiler for rc files
IF (MINGW)
  set(CMAKE_RC_COMPILER_INIT windres)
  ENABLE_LANGUAGE(RC)
  SET(CMAKE_RC_COMPILE_OBJECT
    "<CMAKE_RC_COMPILER> --output-format=coff <FLAGS> <DEFINES> -o <OBJECT> <SOURCE>")
ENDIF (MINGW)

# translations
QT4_CREATE_TRANSLATION(
    qpamat_qmfile
    ${CMAKE_SOURCE_DIR}/src
    ts/de.ts
)

# Compiled-in pixmaps
SET(QtApp_RCCS qpamat.qrc)


# generate rules for building source files from the resources
QT4_ADD_RESOURCES(qpamat_RCC_SRCS qpamat.qrc)

# generate rules for building source files that moc generates
QT4_WRAP_CPP(qpamat_MOC_SRCS ${qpamat_MOCS})

# build sources, moc'd sources, and rcc'd sources
ADD_EXECUTABLE(qpamat WIN32
    ${qpamat_SRCS} ${qpamat_MOC_SRCS} ${qpamat_RCC_SRCS} ${qpamat_qmfile}
)

SET(EXTRA_LIBS ${QT_LIBRARIES} ${OPENSSL_LIBRARIES})
IF (X11_FOUND)
    SET (EXTRA_LIBS ${EXTRA_LIBS} ${X11_LIBRARIES})
ENDIF (X11_FOUND)

TARGET_LINK_LIBRARIES(qpamat ${EXTRA_LIBS})

# apidoc
ADD_CUSTOM_TARGET(
    apidoc
    doxygen ${CMAKE_SOURCE_DIR}/doxyfile
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Generating API documentation"
)

#
# {{{ Tests
#

IF (BUILD_TESTING)

    #
    # Securestring
    #
    SET(testsecurestring_SRCS
        src/util/securestring.cpp
        src/tests/securestring.cpp
    )

    SET(testsecurestring_MOCS
        src/tests/securestring.h
    )

    QT4_WRAP_CPP(testsecurestring_MOC_SRCS ${testsecurestring_MOCS})
    ADD_EXECUTABLE(testsecurestring
        ${testsecurestring_SRCS}
        ${testsecurestring_MOCS}
        ${testsecurestring_MOC_SRCS}
    )
    TARGET_LINK_LIBRARIES(testsecurestring
        ${QT_LIBRARIES}
    )

    #
    # Logging
    #

    # FIXME: Build currently only works on POSIX.
    # Create a static library and build both the test cases and the main
    # program from that static library.
    SET(logtest_SRCS
        src/util/debug.cpp
        src/util/ansicolor.cpp
        src/util/msghandler.cpp
        src/util/platformhelpers_posix.cpp
        src/tests/logtest.cpp
    )

    ADD_EXECUTABLE(logtest ${logtest_SRCS})
    TARGET_LINK_LIBRARIES(logtest ${QT_LIBRARIES})
ENDIF (BUILD_TESTING)

ADD_TEST(SecureString testsecurestring)

# }}}

#
# {{{ Build the Documentation
#

FIND_PROGRAM(XSLTPROC_EXECUTABLE xsltproc)

# create target directories
FILE(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/doc/user/en)

ADD_CUSTOM_COMMAND(
    OUTPUT
        ${CMAKE_BINARY_DIR}/doc/user/en/index.html # creates more files
    COMMAND
        ${XSLTPROC_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/doc/user/en/qpamat.xsl
        ${CMAKE_SOURCE_DIR}/doc/user/en/qpamat.xml &&
        ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/doc/user/en/qpamat.css
        ${CMAKE_BINARY_DIR}/doc/user/en/qpamat.css
    DEPENDS
        ${CMAKE_SOURCE_DIR}/doc/user/en/qpamat.xml
    WORKING_DIRECTORY
        ${CMAKE_BINARY_DIR}/doc/user/en
)

ADD_CUSTOM_TARGET(
    doc
    DEPENDS
        ${CMAKE_BINARY_DIR}/doc/user/en/index.html
)

ADD_DEPENDENCIES(
    qpamat
    doc
)

# }}}

#
# {{{ Installation
#

INSTALL(
    TARGETS
        qpamat
    DESTINATION
        bin
)

INSTALL(
    FILES
        ${qpamat_qmfile}
    DESTINATION
        share/qpamat/translations
)

INSTALL(
    FILES
        ${CMAKE_SOURCE_DIR}/share/dicts/all.txt
        ${CMAKE_SOURCE_DIR}/share/dicts/english.txt
        ${CMAKE_SOURCE_DIR}/share/dicts/french.txt
        ${CMAKE_SOURCE_DIR}/share/dicts/german.txt
        ${CMAKE_SOURCE_DIR}/share/dicts/netherlands.txt
        ${CMAKE_SOURCE_DIR}/share/dicts/default.txt
    DESTINATION
        share/qpamat/dicts/
)

INSTALL(
    FILES
        ${CMAKE_SOURCE_DIR}/images/qpamat_48.png
    DESTINATION
        share/pixmaps/
    RENAME
        qpamat.png
)

INSTALL(
    FILES
        ${CMAKE_SOURCE_DIR}/share/qpamat.desktop
    DESTINATION
        share/applications
)

INSTALL(
    DIRECTORY
        ${CMAKE_BINARY_DIR}/doc/user/en
    DESTINATION
        share/qpamat/doc/
    FILES_MATCHING REGEX ".*.(html|css)"
)

# }}}

#
# {{{  Installation
#

CONFIGURE_FILE(
    "${CMAKE_SOURCE_DIR}/share/macos/Info.plist.in"
    "${CMAKE_BINARY_DIR}/share/macos/Info.plist"
)

IF (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    SET(CPACK_GENERATOR "Bundle")
    SET(CPACK_PACKAGE_FILE_NAME "qpamat-${VERSION_STRING}")
    #SET(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/images/qpamat.icns")
    SET(CPACK_BUNDLE_NAME "QPaMaT")
    SET(CPACK_BUNDLE_PLIST "${CMAKE_BINARY_DIR}/share/macos/Info.plist")
    SET(CPACK_BUNDLE_ICON "${CMAKE_SOURCE_DIR}/images/qpamat.icns")
    SET(CPACK_BUNDLE_STARTUP_COMMAND qpamat)
    INCLUDE(CPack)
ENDIF (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")

# }}}

# vim: set sw=4 ts=4 et fdm=marker:
