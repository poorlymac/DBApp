/* WARNING: Do NOT forget to terminate your lines with ; otherwise it breaks in strange ways */
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
                                    txt += "<tr><th>" + tabinfo.data[x].table_name + "</th><td>" + tabinfo.data[x].table_type + "</td><td>" + tabinfo.data[x].engine + "</td><td data-sort=\"" + tr + "\">" + tr.toLocaleString() + "</td><td data-sort=\"" + dl + "\">" + dl.toLocaleString() + "</td><td data-sort=\"" + il + "\">" + il.toLocaleString() + "</td><td data-sort=\"" + di + "\">" + di.toLocaleString() + "</td><td>" + tabinfo.data[x].table_comment + "</td></tr>";
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
