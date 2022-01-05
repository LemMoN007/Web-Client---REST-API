# WebClient using REST-API
In implementare am folosit biblioteca `parson`. Pentru o apelare mai usoara
si pentru a mai reduce din cod am creat 3 functii ajutatoare `send_get_message`,
`send_post_message` si `send_delete_message` ce se ocupa de stabilirea conexiunii si 
transmiterea comenzilor catre server. Pentru fiecare comanda data de la tastatura (register,
login, get_books, ...) am creat cate o functie:
* register_user -> Citesc username-ul si parola introduse de utilizator, creez
	un obiect `JSON` cu acestea si folosesc functia ajutatoare pentru a trimite comanda
	server-ului. Verific cu ajutorul bibliotecii `parson` daca a fost intors un mesaj
	de eroare, caz in care il afisez.
* login -> Daca comanda de `login` este data de un utilizator deja logat atunci afisez un
	mesaj de eroare. Citesc username-ul si parola introduse de utilizator, creez
	un obiect `JSON` cu acestea si folosesc functia ajutatoare pentru a trimite comanda
	server-ului. Verific daca raspunsul primit este o eroare, altfel logarea s-a efectuat
	cu succes si retin cookie-ul.
* enter_library -> Daca utilizatorul are deja token de acces atunci afisez mesaj de eroare.
	Trimit o cerere de `GET` server-ului, verific daca raspunsul intors este un mesaj de eroare,
	caz in care il afisez, altfel comanda s-a efectuat cu succes si retin tokenul de autentificare.
* get_books -> Timit o cerere de `GET` server-ului si din raspunsul primit creez un `JSON_Array`. 
	Daca nu sunt carti in biblioteca atunci afisez un mesaj, altfel parcurg fiecare obiect
	din array si il afisez.
* get_book -> Citesc id-ul introdus de user, construiesc noul url si apoi trimit o cerere de `GET` 
	server-ului. Verific daca raspunsul intors este o eroare, caz in care o afisez, altfel
	afisez detaliile cartii.
* add_book -> Verific daca utilizatorul este logat si are token-ul de acces pentru biblioteca.
	Citesc datele introduse de user si construiesc un obiect `JSON` cu ele, pe care il folosesc
	in cererea de tip `POST` trimisa catre server. Daca raspunsul intors contine o eroare atunci afisez
	un mesaj de eroare, altfel cartea a fost adaugata cu succes in biblioteca.
* delete_book -> Ca si la `get_book`, se citeste id-ul introdus de user, se construieste noul url, se 
	trimite o cerere de `DELETE` server-ului si se verifica ca raspunsul intors sa nu contina erori. Daca
	totul este in regula atunci stergerea cartii din biblioteca a fost efectuata cu succes.
* logout -> Trimit o cerere de `GET` server-ului pentru logout, verific daca a fost intors un mesaj de eroare,
	iar daca totul este ok resetez `cookies`, `cookies_count` si `auth_token` si afisez un mesaj pentru
	delogarea efectuata cu succes.
* exit -> Inchide programul.
