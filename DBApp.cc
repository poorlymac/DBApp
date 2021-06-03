#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <sstream>
#include </usr/local/Cellar/mysql-client/8.0.25/include/mysql/mysql.h>
#include "webview.h"
#include "DBAppHTML.h"

using namespace std;

string encryptDecrypt(string toEncrypt) {
    string key = "Wynona'sGotHerselfABigBrownBeaverAndSheShowsItOffToAllHerFriends";
    string output = toEncrypt;
    for (string::size_type i = 0; i < toEncrypt.size(); i++) {
        output[i] = toEncrypt[i] ^ key[i];
    }
    return output;
}

std::string ReplaceString(std::string subject, const std::string& search, const std::string& replace) {
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos) {
         subject.replace(pos, search.length(), replace);
         pos += replace.length();
    }
    return subject;
}

void hexchar(unsigned char c, unsigned char &hex1, unsigned char &hex2)
{
    hex1 = c / 16;
    hex2 = c % 16;
    hex1 += hex1 <= 9 ? '0' : 'a' - 10;
    hex2 += hex2 <= 9 ? '0' : 'a' - 10;
}

string urlencode(string s) {
    const char *str = s.c_str();
    vector<char> v(s.size());
    v.clear();
    for (size_t i = 0, l = s.size(); i < l; i++) {
        char c = str[i];
        if ((c >= '0' && c <= '9') ||
            (c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            c == '-' || c == '_' || c == '.' || c == '!' || c == '~' ||
            c == '*' || c == '\'' || c == '(' || c == ')')
        {
            v.push_back(c);
        } else if (c == ' ') {
            v.push_back('+');
        } else {
            v.push_back('%');
            unsigned char d1, d2;
            hexchar(c, d1, d2);
            v.push_back(d1);
            v.push_back(d2);
        }
    }
    return string(v.cbegin(), v.cend());
}

// Helpers to avoid too much typing
id operator"" _cls(const char *s, std::size_t) { return (id)objc_getClass(s); }
SEL operator"" _sel(const char *s, std::size_t) { return sel_registerName(s); }
id operator"" _str(const char *s, std::size_t) {
    return ((id(*)(id, SEL, const char *))objc_msgSend)("NSString"_cls, "stringWithUTF8String:"_sel, s);
}

static id create_menu_item(id title, const char *action, const char *key) {
    #ifdef __APPLE__
        id item = ((id(*)(id, SEL))objc_msgSend)((id)"NSMenuItem"_cls, "alloc"_sel);
        ((id(*)(id, SEL, id, SEL, id))objc_msgSend)(
            item,
            "initWithTitle:action:keyEquivalent:"_sel,
            title,
            sel_registerName(action),
            ((id(*)(id, SEL, const char *))objc_msgSend)("NSString"_cls, "stringWithUTF8String:"_sel, key)
        );
        ((id(*)(id, SEL))objc_msgSend)(item, "autorelease"_sel);
        return item;
    #else
        return NULL;
    #endif
}

std::string getEnvVar(std::string const& key) {
    char const* val = getenv(key.c_str()); 
    return val == NULL ? std::string() : std::string(val);
}

std::string cnf;
MYSQL_RES *result;
MYSQL_ROW row;
MYSQL *connection, mysql;

void signalHandler(int signum) {
    if (connection != NULL) {
        std::cout << "Closing Database Connection" << std::endl;
        mysql_close(connection);
        exit(signum);
    }
}
void functionExit(void) {
    signalHandler(0);
}

#ifdef WIN32
int WINAPI WinMain(HINSTANCE hInt, HINSTANCE hPrevInst, LPSTR lpCmdLine,int nCmdShow) {
#else
int main() {
#endif
    signal(SIGINT, signalHandler);
    atexit (functionExit);
    cnf = getEnvVar("HOME") +  "/.DBApp.cnf";
    #ifdef __APPLE__
        // Thanks goes to https://github.com/lukevers/webview/blob/edit-menu/webview.h fork for all of this section

        /*** Create menubar ***/
        id menubar = ((id(*)(id, SEL))objc_msgSend)((id)"NSMenu"_cls, "alloc"_sel);
        ((id(*)(id, SEL, id))objc_msgSend)(menubar, "initWithTitle:"_sel, ""_str);
        ((id(*)(id, SEL))objc_msgSend)(menubar, "autorelease"_sel);

        id appName = ((id(*)(id, SEL))objc_msgSend)(
        ((id(*)(id, SEL))objc_msgSend)((id)"NSProcessInfo"_cls, "processInfo"_sel),
        "processName"_sel
        );

        /*** Main application menu ***/
        id appMenuItem = ((id(*)(id, SEL))objc_msgSend)((id)"NSMenuItem"_cls, "alloc"_sel);
        ((id(*)(id, SEL, id, id, id))objc_msgSend)(
            appMenuItem,
            "initWithTitle:action:keyEquivalent:"_sel,
            appName,
            NULL,
            ""_str
        );

        id appMenu = ((id(*)(id, SEL))objc_msgSend)((id)"NSMenu"_cls, "alloc"_sel);
        ((id(*)(id, SEL, id))objc_msgSend)(appMenu, "initWithTitle:"_sel, appName);
        ((id(*)(id, SEL))objc_msgSend)(appMenu, "autorelease"_sel);

        ((id(*)(id, SEL, id))objc_msgSend)(appMenuItem, "setSubmenu:"_sel, appMenu);
        ((id(*)(id, SEL, id))objc_msgSend)(menubar,"addItem:"_sel, appMenuItem);

        id title = ((id(*)(id, SEL, id))objc_msgSend)("Hide "_str, "stringByAppendingString:"_sel, appName);
        id item = create_menu_item(title, "hide:", "h");
        ((id(*)(id, SEL, id))objc_msgSend)(appMenu, "addItem:"_sel, item);

        title = ((id(*)(id, SEL, id))objc_msgSend)(
            "Quit "_str,
            "stringByAppendingString:"_sel,
            appName
        );
        item = create_menu_item(title, "terminate:", "q");
        ((id(*)(id, SEL, id))objc_msgSend)(appMenu, "addItem:"_sel, item);

        /*** Edit menu ***/
        id editMenuItem = ((id(*)(id, SEL))objc_msgSend)((id)"NSMenuItem"_cls, "alloc"_sel);
        ((id(*)(id, SEL, id, id, id))objc_msgSend)(
            editMenuItem,
            "initWithTitle:action:keyEquivalent:"_sel,
            "Edit"_str,
            NULL,
            ""_str
        );

        id editMenu = ((id(*)(id, SEL))objc_msgSend)((id)"NSMenu"_cls, "alloc"_sel);
        ((id(*)(id, SEL, id))objc_msgSend)(editMenu, "initWithTitle:"_sel, "Edit"_str);
        ((id(*)(id, SEL))objc_msgSend)(editMenu, "autorelease"_sel);

        ((id(*)(id, SEL, id))objc_msgSend)(editMenuItem, "setSubmenu:"_sel, editMenu);
        ((id(*)(id, SEL, id))objc_msgSend)(menubar, "addItem:"_sel, editMenuItem);

        item = create_menu_item("Cut"_str, "cut:", "x");
        ((id(*)(id, SEL, id))objc_msgSend)(editMenu, "addItem:"_sel, item);

        item = create_menu_item("Copy"_str, "copy:", "c");
        ((id(*)(id, SEL, id))objc_msgSend)(editMenu, "addItem:"_sel, item);

        item = create_menu_item("Paste"_str, "paste:", "v");
        ((id(*)(id, SEL, id))objc_msgSend)(editMenu, "addItem:"_sel, item);

        item = create_menu_item("Select All"_str, "selectAll:", "a");
        ((id(*)(id, SEL, id))objc_msgSend)(editMenu, "addItem:"_sel, item);
        
        ((id(*)(id, SEL, id))objc_msgSend)(
            ((id(*)(id, SEL))objc_msgSend)((id)"NSApplication"_cls, "sharedApplication"_sel),
            "setMainMenu:"_sel,
            menubar
        );
    #endif

    mysql_init(&mysql);
    webview::webview w(true, nullptr);
    w.set_title("DBApp");
    w.set_size(1280, 1024, WEBVIEW_HINT_NONE);
    w.set_size(180, 120, WEBVIEW_HINT_MIN);

    // login function
    w.bind("wvlogin", [](std::string s) -> std::string {
        auto db = webview::json_parse(s, "", 0);
        // std::cout << s << std::endl;
        auto server   = webview::json_parse(db, "server", 0);
        auto login    = webview::json_parse(db, "login", 1);
        auto password = webview::json_parse(db, "password", 2);

        // Write config to file
        ofstream myconf;
        myconf.open(cnf);
        myconf << "{\"server\":" +  webview::json_escape(server) + ", \"login\":" + webview::json_escape(login) + ", \"password\":" + webview::json_escape(webview::url_encode(encryptDecrypt(password))) + "}";
        myconf.close();

        // TODO MySQL Login
        connection = mysql_real_connect(&mysql, server.c_str(), login.c_str(), password.c_str(), "mysql", 0, 0, 0);
        if (connection == NULL) {
            return "{\"message\":\"Error logging into " + server + " as " + login + ": " + mysql_error(&mysql) + "\", \"result\":false}";
        } else {
            // Get the list of databases
            int state = mysql_query(connection, "SELECT CATALOG_NAME, SCHEMA_NAME, DEFAULT_CHARACTER_SET_NAME, DEFAULT_COLLATION_NAME, COALESCE(ROUND(SUM(data_length + index_length) / 1024 / 1024, 2), 0) AS MB, COUNT(*) TABLES FROM information_schema.schemata s LEFT OUTER JOIN information_schema.tables t ON t.table_schema = s.schema_name GROUP BY CATALOG_NAME, SCHEMA_NAME, DEFAULT_CHARACTER_SET_NAME, DEFAULT_COLLATION_NAME ORDER BY CATALOG_NAME, SCHEMA_NAME");
            if (state !=0) {
                return "{\"message\":\"Error getting schema information: " + std::string(mysql_error(connection)) + "\", \"result\":false}";
            }
            result = mysql_store_result(connection);
            stringstream datastream;
            double mb_max = 0.0;
            double mb_tot = 0.0;
            int tables_tot = 0;
            if (mysql_num_rows(result) > 0) {
                while ( ( row = mysql_fetch_row(result)) != NULL ) {
                    string catalog   = row[0];
                    string schema    = row[1];
                    string charset   = row[2];
                    string collation = row[3];
                    double mb        = atof(row[4]);
                    cout << schema + " " << mb <<endl;
                    int tables       = atoi(row[5]);
                    if(mb > mb_max) {
                        mb_max = mb;
                    }
                    mb_tot += mb;
                    tables_tot += tables;
                    datastream << "{\"catalog\":\"" + catalog + "\",\"schema\":\"" + schema + "\",\"charset\":\"" + charset + "\",\"collation\":\"" + collation + "\",\"mb\":\"" << mb << "\",\"tables\":\"" << tables << "\"},";
                }
            }
            string data = datastream.str().substr(0, datastream.str().size() - 1); // truncate the last comma
            mysql_free_result(result);
            cout << mb_max << endl;
            return "{\"message\":\"Logged in to " + server + " as " + login + "\", \"result\":true,\"data\":[" + data + "], \"tables_tot\":\"" + to_string(tables_tot) + "\", \"mb_tot\":\"" + to_string(mb_tot) + "\", \"mb_max\":\"" + to_string(mb_max) + "\"}";
        }
    });

    // login function
    w.bind("wvtables", [](std::string s) -> std::string {
        auto catalog = webview::json_parse(s, "", 0);
        auto schema = webview::json_parse(s, "", 1);

        // Get the list of tables
        // TODO protect from injection
        string sql = "SELECT table_name, table_type, engine, table_rows, data_length, index_length, table_comment FROM information_schema.tables t WHERE table_schema = '" + schema + "' AND table_catalog = '" + catalog + "' ORDER BY table_type, table_name";
        int state = mysql_query(connection, sql.c_str());
        if (state !=0) {
            return "{\"message\":\"Error getting table information: " + std::string(mysql_error(connection)) + "\", \"result\":false}";
        }
        result = mysql_store_result(connection);
        stringstream datastream;
        int data_length_tot = 0;
        int index_length_tot = 0;
        int table_rows_tot = 0;
        if (mysql_num_rows(result) > 0) {
            while ( ( row = mysql_fetch_row(result)) != NULL ) {
                string table_name    = row[0]==NULL?"":row[0];
                string table_type    = row[1]==NULL?"":row[1];
                string engine        = row[2]==NULL?"":row[2];
                int  table_rows    = atoi(row[3]==NULL?"":row[3]);
                int  data_length   = atoi(row[4]==NULL?"":row[4]);
                int  index_length  = atoi(row[5]==NULL?"":row[5]);
                string table_comment = row[6]==NULL?"":row[6];
                data_length_tot += data_length;
                index_length_tot += index_length;
                table_rows_tot += table_rows;
                datastream << "{\"table_name\":\"" + table_name + "\",\"table_type\":\"" + table_type + "\",\"engine\":\"" + engine + "\",\"table_rows\":\"" << table_rows << "\",\"data_length\":\"" << data_length << "\",\"index_length\":\"" << index_length << "\",\"table_comment\":\"" + table_comment + "\"},";
            }
        }
        string data = datastream.str().substr(0, datastream.str().size() - 1); // truncate the last comma
        mysql_free_result(result);
        return "{\"message\":\"Retrieved table information for catalog " + catalog + " schema " + schema + "\", \"result\":true,\"data\":[" + data + "], \"data_length_tot\":\"" + to_string(data_length_tot) + "\", \"index_length_tot\":\"" + to_string(index_length_tot) + "\", \"table_rows_tot\":\"" + to_string(table_rows_tot) + "\"}";
    });

    std::string s(DBAppHTML_html, DBAppHTML_html + DBAppHTML_html_len);
    w.navigate("data:text/html," + urlencode(s));
    ifstream myconf (cnf.c_str());
    if (myconf.is_open()) {
        string line;
        stringstream buffer;
        while ( getline (myconf, line) ) {
            buffer << line;
        }
        myconf.close();
        line = buffer.str();
        auto server   = webview::json_parse(line, "server", 0);
        auto login    = webview::json_parse(line, "login", 0);
        auto password = encryptDecrypt(webview::url_decode(webview::json_parse(line, "password", 0)));
        w.init("var server = " + webview::json_escape(server) + ";");
        w.init("var login = " + webview::json_escape(login) + ";");
        w.init("var password = " + webview::json_escape(password) + ";");
    } else {
        w.init("var server = '';");
        w.init("var login = '';");
        w.init("var password = '';");
    }
    w.run();
    std::cout << "Closing Database Connection" << std::endl;
    mysql_close(connection);
    return 0;
}
