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

•	Omdat het multithreaden van het updaten van tanks geen snelheidsvoordeel bood, is hieronder een code snippet van dit gedeelte opgenomen voor referentie.

```c++
void update_tank(Tank& tank, vector<Tank>& tanks, Terrain& background_terrain, vector<Rocket>& rockets, float rocket_radius, Sprite& rocket_red, Sprite& rocket_blue)
{
    //Move tanks according to speed and nudges (see above) also reload
    tank.tick(background_terrain);

    //Shoot at closest target if reloaded
    if (tank.rocket_reloaded())
    {
        Tank& target = Tank::find_closest_enemy(tank, tanks);

        rockets_mutex.lock();
        rockets.push_back(Rocket(tank.position, (target.get_position() - tank.position).normalized() * 3, rocket_radius, tank.allignment, ((tank.allignment == RED) ? &rocket_red : &rocket_blue)));
        rockets_mutex.unlock();

        tank.reload_rocket();
    }
}

void Tank::update_tanks(ThreadPool* pool, std::vector<std::future<void>>& futures, vector<Tank>& tanks, Terrain& background_terrain, vector<Rocket>& rockets, float rocket_radius, Sprite& rocket_red, Sprite& rocket_blue)
{
    futures.clear();
    futures.reserve(tanks.size());

    
    for (Tank& tank : tanks)
    {
        if (tank.active)
        {
            futures.push_back(pool->enqueue([&tank, &tanks, &background_terrain, &rockets, rocket_radius, &rocket_red, &rocket_blue]() { update_tank(tank, tanks, background_terrain, rockets, rocket_radius, rocket_red, rocket_blue); }));
        }
    }
    for (auto& future : futures)
    {
        future.get();
    }
}
```
