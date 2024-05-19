# special-octo-spork
Exemple de code C, qui implémente un scheduler qui gère plusieurs thread à tour de rôle. Premièrement en utilisant plusieur thread et deuxièmement en utilisant la sauvegarde et la restitution de contexte

Objectif: pthread, context et scheduling basique

-Créer deux threads avec pthread. Chaque thread s'exécute indéfiniment et affiche son nom toutes les 3 secondes.
-créer un troisième thread avec pthread qui joue le rôle de scheduler. Ce dernier se reveille toutes les 5 secondes et interrope l'un des deux threads en cours d'exécution. Ensuite, il débloque le thread précédemment bloqué. Le scheduler attend 10 secondes avant de commencer son scheduler périodique.

Note:
Les threads crées avec pthread sont vus par l'OS.
Ils sont donc schedulés par le scheduler l'OS comme tout processus.
Ainsi, pendant les 10 premières secondes, où votre scheduler n'intervient pas, les deux threads vont s'exécuter en parallèle (car schedulés par le scheduler de l'OS).
Après les 10 première seconde, votre scheduler forcera une exécution alternée.
