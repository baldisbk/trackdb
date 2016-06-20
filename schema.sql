create table track (
	id integer primary key autoincrement,
	title text,
	artist text,
	album text,
	main integer null,
	minus integer null,
	lastchange text
	);

create table file (
	id integer primary key autoincrement,
	filename text,
	title text,
	artist text,
	album text,
	track integer null,
	lastplayed text
	);

create table property (
	id integer primary key autoincrement,
	name text,
	big boolean,
	fixed boolean
	);

create table tag (
	id integer primary key autoincrement,
	tag text,
	property integer null
	);

create table tracktag (
	track integer not null,
	tag integer not null
	);

create table trackproperty (
	track integer not null,
	property integer not null,
	value text
	);

insert into property (name, big, fixed) values
	("Grammar",0,1),("Lexics",0,1),("Difficulty",0,1),("Vocal",0,1),("Age",0,1),
	("Year",0,0),
	("English",1,0),("Russian",1,0),("Comment",1,0),("History",1,0),("Exercises",1,0);
