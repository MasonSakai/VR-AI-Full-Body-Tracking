const { Console } = require('console');
const http = require('http');
const fs = require('fs');

const hostname = '127.0.0.1';
const port = 1337;

const ContentTypes = {
	"html": "text/html",
	"js": "application/javascript",
	"css": "text/css"
}

const server = http.createServer((req, res) => {
	try {
		var url = req.url;
		console.log(`URL: ${url}`);
		if (url == "/") url = "dist/index.html";
		else url = "dist" + url;
		var data = fs.readFileSync(url, 'utf8');

		var splits = url.split('.');
		var contentType = ContentTypes[splits[splits.length - 1]];
		console.log(contentType);

		res.statusCode = 200;
		res.setHeader('Content-Type', contentType);
		res.end(data);
	} catch (err) {
		console.log(err)
		res.statusCode = 404;
		res.setHeader('Content-Type', 'text/plain');
		res.end("File Not Found");
	}
});

server.listen(port, hostname, () => {
	console.log(`Server running at http://${hostname}:${port}/`);
});