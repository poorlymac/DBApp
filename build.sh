# Get the most recent webview.h
curl -O https://raw.githubusercontent.com/webview/webview/master/webview.h
# get the most recent sortable js and css
curl -O https://raw.githubusercontent.com/tofsjonas/sortable/main/sortable.js
curl -O https://raw.githubusercontent.com/tofsjonas/sortable/main/sortable.css

# Convert HTML's to header files
echo "<style>" > DBAppHTML.html
cat sortable.css >> DBAppHTML.html
echo "</style>" >> DBAppHTML.html
echo "<script language=\"JavaScript\">" >> DBAppHTML.html
cat sortable.js >> DBAppHTML.html
echo "</script>" >> DBAppHTML.html
cat DBApp.html >> DBAppHTML.html
xxd -i DBAppHTML.html > DBAppHTML.h
rm DBAppHTML.html

# Compile
if [ "$(uname)" == "Darwin" ]; then
    rm -f DBApp.app/Contents/MacOS/DBApp
    # Statically compile in mysql client, ssl and crypto stuff
    c++ DBApp.cc -std=c++11 -framework WebKit -I/usr/local/include -I/usr/local/include/mysql -W  -L/usr/local/lib -lz /usr/local/lib/libzstd.a /usr/local/lib/libmysqlclient.a /usr/local/Cellar/openssl@1.1/1.1.1k/lib/libcrypto.a /usr/local/Cellar/openssl@1.1/1.1.1k/lib/libssl.a -o DBApp.app/Contents/MacOS/DBApp
    if [ $? -eq 0 ]
    then
        ls -lah DBApp.app/Contents/MacOS/DBApp
        ibtool --compile DBApp.app/Contents/Resources/MainMenu.nib DBApp.app/Contents/Resources/MainMenu.xib
        if [ $? -eq 0 ]
        then
            #DBApp.app/Contents/MacOS/DBApp # good for testing
            open DBApp.app
        fi
    fi
elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
    rm -f DBApp
    c++ DBApp.cc `pkg-config --cflags --libs gtk+-3.0 webkit2gtk-4.0` -I/usr/local/include -I/usr/local/include/mysql -W -lmysqlclient -L/usr/local/lib -static -o DBApp
    if [ $? -eq 0 ]
    then
        ls -lah DBApp
        ./DBApp
    fi
elif [ "$(expr substr $(uname -s) 1 10)" == "MINGW32_NT" ]; then
    DEL DBApp.exe
    c++ DBApp.cc -mwindows -L./dll/x32 -lwebview -lWebView2Loader -o DBApp.exe
    if [ $? -eq 0 ]
    then
        DIR "DBApp.exe"
        DBApp.exe
    fi
elif [ "$(expr substr $(uname -s) 1 10)" == "MINGW64_NT" ]; then
    DEL DBApp.exe
    c++ DBApp.cc -mwindows -L./dll/x64 -lwebview -lWebView2Loader -o DBApp.exe
    if [ $? -eq 0 ]
    then
        DIR "DBApp.exe"
        DBApp.exe
    fi
fi