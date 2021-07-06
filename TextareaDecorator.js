/* TextareaDecorator.js
 * written by Colin Kuebler 2012
 * Part of LDT, dual licensed under GPLv3 and MIT
 * Builds and maintains a styled output layer under a textarea input layer
 */

function TextareaDecorator( textarea, parser ){
	/* INIT */
	var api = this;

	var parent = document.createElement("div");
	var output = document.createElement("pre");
	parent.appendChild(output);
	var label = document.createElement("label");
	parent.appendChild(label);
	textarea.parentNode.replaceChild( parent, textarea );
	label.appendChild(textarea);
	parent.className = 'ldt ' + textarea.className;
	textarea.className = '';
	textarea.spellcheck = false;
	textarea.wrap = "off";

	var color = function( input, output, parser ){
		var oldTokens = output.childNodes;
		var newTokens = parser.tokenize(input);
		var firstDiff, lastDiffNew, lastDiffOld;
		for( firstDiff = 0; firstDiff < newTokens.length && firstDiff < oldTokens.length; firstDiff++ )
			if( newTokens[firstDiff] !== oldTokens[firstDiff].textContent ) break;
		while( newTokens.length < oldTokens.length )
			output.removeChild(oldTokens[firstDiff]);
		for( lastDiffNew = newTokens.length-1, lastDiffOld = oldTokens.length-1; firstDiff < lastDiffOld; lastDiffNew--, lastDiffOld-- )
			if( newTokens[lastDiffNew] !== oldTokens[lastDiffOld].textContent ) break;
		for( ; firstDiff <= lastDiffOld; firstDiff++ ){
			oldTokens[firstDiff].className = parser.identify(newTokens[firstDiff]);
			oldTokens[firstDiff].textContent = oldTokens[firstDiff].innerText = newTokens[firstDiff];
		}
		for( var insertionPt = oldTokens[firstDiff] || null; firstDiff <= lastDiffNew; firstDiff++ ){
			var span = document.createElement("span");
			span.className = parser.identify(newTokens[firstDiff]);
			span.textContent = span.innerText = newTokens[firstDiff];
			output.insertBefore( span, insertionPt );
		}
	};

	api.input = textarea;
	api.output = output;
	api.update = function(){
		var input = textarea.value;
		if( input ){
			color( input, output, parser );
			var lines = input.split('\n');
			var maxlen = 0, curlen;
			for( var i = 0; i < lines.length; i++ ){
				var tabLength = 0, offset = -1;
				while( (offset = lines[i].indexOf( '\t', offset+1 )) > -1 ){
					tabLength += 7 - (tabLength + offset) % 8;
				}
				var curlen = lines[i].length + tabLength;
				maxlen = maxlen > curlen ? maxlen : curlen;
			}
			textarea.cols = maxlen + 1;
			textarea.rows = lines.length + 2;
		} else {
			output.innerHTML = '';
			textarea.cols = textarea.rows = 1;
		}
	};

	if( textarea.addEventListener ){
		textarea.addEventListener( "input", api.update, false );
	} else {
		textarea.attachEvent( "onpropertychange",
			function(e){
				if( e.propertyName.toLowerCase() === 'value' ){
					api.update();
				}
			}
		);
	}
	api.update();

	return api;
};

