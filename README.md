# FlashDrive - Optimalisatie Project

Project FlashDrive is een geoptimaliseerde versie van een bestaande tank simulatie.

## Werkwijze
•	De codebase is gereorganiseerd door functies te splitsen en ze te groeperen per bijbehorende klassen, wat de structuur overzichtelijker maakt.

•	Via een C++ profiler zijn de meest tijdrovende functies geïdentificeerd tijdens simulaties van 2000 frames.

•	Optimalisatiemogelijkheden zijn onderzocht op basis van de huidige algoritmen. Hierbij is alleen gekeken naar de tijdcomplexiteit, met de nadruk op het verbeteren van de efficiëntie. 

## Algoritmen
•	Voor een snellere collision detection tussen tanks is een Kd-tree geïmplementeerd, waardoor het zoekproces van voor elke tank elke andere tank controleren ( **O(n^2)** ) naar gemiddeld **O(log n)** is verbeterd. Dit komt doordat tanks efficiënter worden georganiseerd in de boomstructuur.

•	Dijkstra's algoritme is ingezet voor het bepalen van snelste routes in een grid met obstakels, waarbij de tijdcomplexiteit van **O((V+E) log V)** is toegepast. Hierbij staat V voor het aantal knooppunten (celposities) en E voor de randen (mogelijke bewegingen) in het grid. Dit vervangt het BFS-algoritme met een complexiteit van **O(N×M)**, waarbij N en M de dimensies van het grid zijn.

•	Het sorteren van healthbars is geüpgraded van Insertion Sort (**O(n^2)**) naar Quick Sort (**O(n log n)**), waarbij QuickSort efficiënt werkt door de array te verdelen en te sorteren rond gekozen pivot elementen.

## Multithreaden
•	Het multithreaden van het updaten van rockets heeft een significante snelheidsverbetering opgeleverd, met een gemiddelde speedup van 3,3 na alle verbeteringen. Aanvankelijk was de speedup 1,0 en na de algoritmische verbeteringen steeg deze naar 2,2.

•	Ondanks dat het multithreaden van het updaten van tanks geen snelheidsvoordeel bood, is de code behouden voor beoordeling.

