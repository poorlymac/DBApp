/* WARNING: Do NOT forget to terminate your lines with ; otherwise it breaks in strange ways */
/* WARNING: use /* style commenting */

var schemaData;

function doLogin() {
    document.getElementById('loginButton').disabled = true;
    var db = {
        server:   document.getElementById('server').value,
        login:    document.getElementById('login').value,
        password: document.getElementById('password').value
    };
    server = document.getElementById('server').value;
    login = document.getElementById('login').value;
    password = document.getElementById('password').value;
    wvlogin(
        db
    ).then(
        function(res) {
            document.getElementById('loginButton').disabled = false;
            document.getElementById('message').innerHTML = res.message;
            if (res.result) {
                document.getElementById('loginview').style.visibility = "hidden";
                document.getElementById('dbview').style.visibility = "visible";
                document.getElementById('header').innerHTML = "Connected to mysql://" + login + "@" + server + "/ Version: " + res.version + " Schemas: " + parseInt(res.data.length).toLocaleString() + ", Tables: " + parseInt(res.tables_tot).toLocaleString() + ", Views: " + parseInt(res.views_tot).toLocaleString() + ", MB: " + parseFloat(res.mb_tot).toLocaleString() + " <button onclick=\"doLogout()\">Logout</button>";
                document.getElementById('footer').innerHTML = res.message;
                schemaData = res.data;
                var schemas = document.getElementById('schemas');
                document.getElementById('schemas').length = 0;
                for (var i = 0; i < res.data.length; i = i + 1) {
                    var schema = document.createElement("li");
                    var a = document.createElement('a');
                    var ap = document.createElement('a');
                    var progress = document.createElement('progress');
                    progress.style.width = "50px";
                    progress.max = parseInt(res.mb_max);
                    progress.value = parseInt(res.data[i].mb);
                    var linkText = document.createTextNode(res.data[i].schema);
                    ap.appendChild(progress);
                    ap.href = "javascript:void(0)";
                    ap.title = parseFloat(res.data[i].mb).toLocaleString() + "MB";
                    schema.appendChild(ap);
                    schema.appendChild(document.createTextNode(" "));
                    a.appendChild(linkText);
                    a.href = "javascript:void(0)";
                    a.sn = i;
                    a.onclick = function () {
                        document.getElementById('catalog').value = schemaData[this.sn].catalog;
                        document.getElementById('schema').value = schemaData[this.sn].schema;
                        document.getElementById('content').innerHTML = "<table class='sortable'><thead><th>Catalogue</th><th>Schema</th><th>Charset</th><th>Collation</th><th>Size (MB)</th><th>Tables</th></tr><tbody><tr><td>" + schemaData[this.sn].catalog + "</td><td>" + schemaData[this.sn].schema + "</td><td>" + schemaData[this.sn].charset + "</td><td>" + schemaData[this.sn].collation + "</td><td>" + parseFloat(schemaData[this.sn].mb).toLocaleString() + "</td><td>" + parseInt(schemaData[this.sn].tables).toLocaleString() + "</td></tr></tbody></table>";
                        wvtables(
                            schemaData[this.sn].catalog,
                            schemaData[this.sn].schema
                        ).then(
                            function(tabinfo) {
                                var txt = "<table class='sortable'><thead><th>Table Name</th><th>Type</th><th>Engine</th><th>Rows</th><th>Data (MB)</th><th>Index (MB)</th><th>Total (MB)</th><th>Comment</th></thead></tr><tbody>";
                                for (var x in tabinfo.data) {
                                    var tr = parseFloat(tabinfo.data[x].table_rows);
                                    var dl = parseFloat(tabinfo.data[x].data_length);
                                    var il = parseFloat(tabinfo.data[x].index_length);
                                    var di = dl + il;
                                    txt += "<tr><th>" + tabinfo.data[x].table_name + "</th><td>" + tabinfo.data[x].table_type + "</td><td>" + tabinfo.data[x].engine + "</td><td style=\"text-align:right\" data-sort=\"" + tr + "\">" + tr.toLocaleString() + "</td><td style=\"text-align:right\" data-sort=\"" + dl + "\">" + dl.toLocaleString() + "</td><td style=\"text-align:right\" data-sort=\"" + il + "\">" + il.toLocaleString() + "</td><td style=\"text-align:right\" data-sort=\"" + di + "\">" + di.toLocaleString() + "</td><td>" + tabinfo.data[x].table_comment + "</td></tr>";
                                }
                                txt += "</tbody></table>";
                                document.getElementById('content').innerHTML += ",\n" + txt;
                            }
                        );
                        return false;
                    };
                    schema.appendChild(a);
                    schemas.appendChild(schema);
                }
            }
        }
    );
}

function doLogout() {
    document.getElementById('dbview').style.visibility = "hidden";
    document.getElementById('loginview').style.visibility = "visible";
    document.getElementById('message').innerHTML = "";
    wvlogout().then(
        function(res) {
            document.getElementById('message').innerHTML = res.message;
        }
    );
}

function runSQL() {
    document.getElementById('runsql').disabled = true;
    wvsql(document.getElementById('catalog').value, document.getElementById('schema').value, document.getElementById('editing').value).then(
        function(res) {
            document.getElementById('runsql').disabled = false;
            document.getElementById('result').innerHTML = res.message;
            if (res.result) {
                var txt = "<table class='sortable'><thead>";
                for (var columnname in res.columns.columnname) {
                    txt += "<th>" + columnname + "</th>";
                }
                txt += "</thead></tr><tbody>";
                for (var columns in res.data) {
                    txt += "<tr>";
                    for (var column in res.data[columns]) {
                        txt += "<td>" + res.data[columns][column] + "</td>";
                    }
                    txt += "</tr>";
                }
                txt += "</tbody></table>";
                document.getElementById('result').innerHTML += txt;
            }
        }
    );
}

/* prism code editing support */
function update(text) {
    let result_element = document.querySelector("#highlighting-content");
    /* Update code */
    result_element.innerHTML = text.replace(new RegExp("&", "g"), "&").replace(new RegExp("<", "g"), "<"); /* Global RegExp */
    /* Syntax Highlight */
    Prism.highlightElement(result_element);
}

function sync_scroll(element) {
    /* Scroll result to scroll coords of event - sync with textarea */
    let result_element = document.querySelector("#highlighting");
    /* Get and set x and y */
    result_element.scrollTop = element.scrollTop;
    result_element.scrollLeft = element.scrollLeft;
}
function check_tab(element, event) {
    let code = element.value;
    if(event.key == "Tab") {
        /* Tab key pressed */
        event.preventDefault(); /* stop normal*/
        let before_tab = code.slice(0, element.selectionStart); /* text before tab */
        let after_tab = code.slice(element.selectionEnd, element.value.length); /* text after tab */
        let cursor_pos = element.selectionEnd + 2; /* where cursor moves after tab - 2 for 2 spaces */
        element.value = before_tab + "  " + after_tab; /* add tab char - 2 spaces */
        /* move cursor */
        element.selectionStart = cursor_pos;
        element.selectionEnd = cursor_pos;
    }
}