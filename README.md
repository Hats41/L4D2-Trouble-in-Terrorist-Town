# L4D2 Trouble in Terrorist Town (TTT)

Una recreación completa del popular modo de juego Trouble in Terrorist Town de Garry's Mod para Left 4 Dead 2.

## Características Principales
* **Sistema de Roles:** Al inicio de cada ronda, los jugadores son asignados secretamente como Inocentes, Traidores o Detectives.
* **Ceguera de Roles:** Los Inocentes no saben en quién confiar. Los Traidores conocen a sus compañeros. Los Detectives son revelados públicamente.
* **Secreto y Sigilo:** Barra de vida y paneles de compañeros ocultos (`m_iHideHUD 64`) para evitar que la gente detecte los asesinatos viendo el HUD.
* **Identificación Visual:** Los Traidores y Detectives tienen iconos flotantes sobre sus cabezas (Sprites customizados) que **solo son visibles para sus compañeros de equipo**.
* **Ragdolls Avanzados:** Al morir, los jugadores dejan un cadáver interactivo que los Detectives e Inocentes pueden inspeccionar para obtener información clave.
* **Truce y Respawns:** Sistema de rondas automático, con limpieza de mapa y regeneración de armas entre rondas.
* **Fuego Amigo Real:** Todo el mundo es vulnerable a los ataques de los demás.

## Instalación

1. Descarga el repositorio o haz un archivo `.zip` con todo el contenido.
2. Extrae las carpetas `addons` y `materials` directamente en la carpeta `left4dead2/` de tu servidor.
3. Asegúrate de que tienes instalado **SourceMod** y **Metamod:Source**.
4. ¡Listo! El plugin principal `l4d2_ttt.smx` se cargará automáticamente.

## Configuración y Comandos

* `sm_setrole <nombre> <1|2|3>` - Comando de administrador para forzar el rol de un jugador (1=Inocente, 2=Traidor, 3=Detective).
* `ttt_weapons.cfg` - En este archivo puedes personalizar exactamente qué armas aparecen al inicio de cada ronda y sus coordenadas en el mapa.

## FastDL (Descarga Rápida de Sprites)

Para evitar que tus jugadores vean cuadros rosados por culpa de los iconos (Sprites) faltantes, este mod viene con los materiales listos para usar en FastDL.

**Recomendado (GitHub Pages):**
1. Crea un repositorio en tu GitHub y sube todo este proyecto.
2. Ve a las opciones (`Settings -> Pages`) de tu repositorio en GitHub y activa GitHub Pages para la rama `main`.
3. GitHub te dará un enlace. Ve a tu `server.cfg` y agrega esto:
```cfg
sv_allowdownload 1
sv_downloadurl "https://TU-USUARIO.github.io/TU-REPOSITORIO/"
```

## Requisitos
- Left 4 Dead 2 Dedicado (o Listen Server)
- SourceMod 1.11 o superior
- Tener `sv_pure 0` en tu `server.cfg` si usas mods de la workshop.

---
*Desarrollado para la comunidad competitiva y de rol de L4D2.*
