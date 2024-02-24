#!/bin/bash

# Get the most recent webview.h
#curl -O https://raw.githubusercontent.com/webview/webview/master/webview.h
# get the most recent sortable js and css
# Strip comments as // comments break sthings
curl https://raw.githubusercontent.com/tofsjonas/sortable/main/sortable.js | grep -v "// " > sortable.js
#curl -O https://raw.githubusercontent.com/tofsjonas/sortable/main/sortable.css

# I had to get these once and then remove // style comments as this breaks webview.h
# curl -O https://raw.githubusercontent.com/kueblc/LDT/master/lib/Keybinder.js
# curl -O https://raw.githubusercontent.com/kueblc/LDT/master/lib/Parser.js
# curl -O https://raw.githubusercontent.com/kueblc/LDT/master/lib/SelectHelper.js
# curl -O https://raw.githubusercontent.com/kueblc/LDT/master/lib/TextareaDecorator.css
# curl -O https://raw.githubusercontent.com/kueblc/LDT/master/lib/TextareaDecorator.js

# Convert main HTML into a header file
xxd -i DBApp.html DBApp.h

# Compile
if [ "$(uname)" == "Darwin" ]; then
    rm -rf DBApp.app
    mkdir -p DBApp.app/Contents/MacOS
    mkdir -p DBApp.app/Contents/Resources
    # security find-identity -v -p codesigning
    DEV_ID="7SQZL2Q9TC" # <-- Put your Apple Developer ID in here if you wish to codesign
    rm -f DBApp.app/Contents/MacOS/DBApp
    # Statically compile in mysql client, ssl and crypto stuff
    # -arch x86_64 -arch arm64 ... at some point
    if test -f /usr/local/lib/libmysqlclient.a
    then
        c++ DBApp.cc -mmacosx-version-min=13.0 -std=c++11 -framework WebKit -I/usr/local/include -I/usr/local/include/mysql -W  -L/usr/local/lib -lz /usr/local/lib/libzstd.a /usr/local/lib/libmysqlclient.a /usr/local/Cellar/openssl@3/3.2.1/lib/libcrypto.a /usr/local/Cellar/openssl@3/3.2.1/lib/libssl.a -o DBApp.app/Contents/MacOS/DBApp
    fi
    if test -f /opt/homebrew/lib/libmysqlclient.a
    then
        c++ -v -arch arm64 DBApp.cc -mmacosx-version-min=13.0 -std=c++11 -framework WebKit -I/opt/homebrew/include -I/opt/homebrew/include/mysql -W  -L/opt/homebrew/lib -lz /opt/homebrew/lib/libzstd.dylib /opt/homebrew/lib/libmysqlclient.a /opt/homebrew/Cellar/openssl@3/3.2.1/lib/libcrypto.a /opt/homebrew/Cellar/openssl@3/3.2.1/lib/libssl.a -o DBApp.app/Contents/MacOS/DBApp
    fi
    # Dynamic version
    # c++ DBApp.cc -std=c++11 -framework WebKit -I/usr/local/include -I/usr/local/include/mysql -W  -L/usr/local/lib -lmysqlclient -o DBApp.app/Contents/MacOS/DBApp
    if [ $? -eq 0 ]
    then
        xattr -cr DBApp.app
        ls -lah DBApp.app/Contents/MacOS/DBApp
        ibtool --compile DBApp.app/Contents/Resources/MainMenu.nib DBApp.app/Contents/Resources/MainMenu.xib
        cp sortable.js           DBApp.app/Contents/Resources/
        cp sortable.css          DBApp.app/Contents/Resources/
        cp DBApp.js              DBApp.app/Contents/Resources/
        cp DBApp.css             DBApp.app/Contents/Resources/
        cp DBApp.html            DBApp.app/Contents/Resources/
        cp Keybinder.js          DBApp.app/Contents/Resources/
        cp Parser.js             DBApp.app/Contents/Resources/
        cp SelectHelper.js       DBApp.app/Contents/Resources/
        cp TextareaDecorator.css DBApp.app/Contents/Resources/
        cp TextareaDecorator.js  DBApp.app/Contents/Resources/
        if [ $? -eq 0 ]
        then
            if [ "$DEV_ID" != "" ]
            then
                codesign -s "$DEV_ID" DBApp.app
            fi
            # DBApp.app/Contents/MacOS/DBApp # good for testing to look at outputs
            open DBApp.app
        fi
    fi
elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
    rm -f DBApp
    c++ DBApp.cc `pkg-config --cflags --libs gtk+-3.0 webkit2gtk-4.0` -W -lmysqlclient -o DBApp
elif [ "$(expr substr $(uname -s) 1 10)" == "MINGW32_NT" ]; then
    DEL DBApp.exe
    curl -O https://github.com/webview/webview/blob/master/dll/x32/WebView2Loader.dll
    curl -O https://github.com/webview/webview/blob/master/dll/x32/webview.dll
    c++ DBApp.cc -mwindows -L./dll/x32 -lwebview -lWebView2Loader -o DBApp.exe
    if [ $? -eq 0 ]
    then
        DIR "DBApp.exe"
        DBApp.exe
    fi
elif [ "$(expr substr $(uname -s) 1 10)" == "MINGW64_NT" ]; then
    DEL DBApp.exe
    MKDIR dll\x64
    CD dll\x64
    curl -O https://github.com/webview/webview/blob/master/dll/x64/WebView2Loader.dll
    curl -O https://github.com/webview/webview/blob/master/dll/x64/webview.dll
    CD ..\..
    c++ DBApp.cc -mwindows -L./dll/x64 -lwebview -lWebView2Loader -o DBApp.exe
    if [ $? -eq 0 ]
    then
        DIR "DBApp.exe"
        DBApp.exe
    fi
fi