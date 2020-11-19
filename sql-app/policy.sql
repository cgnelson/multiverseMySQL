

TABLE :
	posts
ALLOW :
	WHERE ((visibility = 1 OR username = {{username}}) AND class IN (SELECT class FROM people WHERE people.username = {{username}})) 
	OR (class IN (SELECT class FROM people WHERE people.username = {{username}} AND people.position = 'instructor'))
REWRITE username :
	CASE WHEN anon = 1 AND 
	username != {{username}} AND 
	class NOT IN (SELECT class FROM people WHERE people.username = {{username}} AND people.position = 'instructor')
	THEN 'anonymous'
	ELSE username

	