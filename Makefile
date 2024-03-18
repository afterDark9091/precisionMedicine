# Copyright (C) 2019  Jimmy Aguilar Mena

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

all: client.x server.x client2.x server2.x mycert.pem post.x

client.x: client.c
	gcc $^ -o $@

server.x: server.c
	gcc $^ -o $@

post.x: post.c
	gcc $^ -o $@

client2.x: client2.c
	gcc -Wall $^ -L/usr/lib -lssl -lcrypto -o $@

server2.x: server2.c
	gcc -Wall $^ -L/usr/lib -lssl -lcrypto -o $@

mycert.pem:
	openssl req -x509 -nodes -days 365 -newkey rsa:1024 -keyout $@ -out $@

.PHONY: clean

clean:
	rm -rf *.x
