[![GitHub Repo stars](https://img.shields.io/github/stars/HorizonGamesRoland/ActorIO?style=plastic&color=yellow&logo=github)](https://github.com/HorizonGamesRoland/ActorIO/stargazers) [![GitHub License](https://img.shields.io/github/license/HorizonGamesRoland/ActorIO?style=plastic&logo=apache)](https://www.apache.org/licenses/LICENSE-2.0) [![Discord](https://img.shields.io/discord/1337396883366219808?style=plastic&logo=discord&logoColor=white&label=discord&color=%235865F2)](https://discord.gg/t8STNrGcU3) [![Static Badge](https://img.shields.io/badge/Documentation-blue?style=plastic&logo=gitbook&logoColor=white)](https://horizongames.gitbook.io/actorio)

# About

Actor I/O is a level scripting tool for Unreal Engine, featuring an input/output system similar to the **Source Engine** by Valve™ Software. At its heart, the plugin is an event binding system where actions are executed in reaction to an event being triggered. Using the C++ reflection system of Unreal Engine, the system can react to any dynamic delegate be it C++ or blueprint, and essentially call any function by name with parameters.

The plugin is also available on FAB: [Actor IO Level Scripting](https://www.fab.com/listings/b3a3142b-8890-469a-b04f-626c5acf0d2e)

# Usage

In order to use the I/O system you need to add `ActorIOActions` to actors. Actions are the driving force behind the whole system. Each action represents an event binding, which calls the designated function when that event is triggered. Actions exist in the game world as objects, with their lifetime tied to the actor - more specifically the `ActorIOComponent` of the actor.

Go to **Windows → Actor I/O** to open the action editor tab. The editor is broken up into two tabs: Outputs and Inputs. The **Outputs** tab is your main editing environment, it shows all actions that are tied to this actor. You can add new actions to the actor with the "New Action" button.

![Editor Outputs](https://horizongames.gitbook.io/~gitbook/image?url=https%3A%2F%2F1586816513-files.gitbook.io%2F%7E%2Ffiles%2Fv0%2Fb%2Fgitbook-x-prod.appspot.com%2Fo%2Fspaces%252FJUOUKxX4X7uzzyzY7fQb%252Fuploads%252FMJR9nC4dmRw0nYpI3WMw%252FEditorOutputs.jpg%3Falt%3Dmedia%26token%3D042f9963-951f-466c-b200-a832e4ed2b99&width=768&dpr=4&quality=100&sign=8f3ae64f&sv=2)

The **Inputs** tab shows all actions that are targeting this actor. This tab is purely for debugging and overview purposes. You cannot edit or change actions from here, only from the calling actor.

![Editor Inputs](https://horizongames.gitbook.io/~gitbook/image?url=https%3A%2F%2F1586816513-files.gitbook.io%2F%7E%2Ffiles%2Fv0%2Fb%2Fgitbook-x-prod.appspot.com%2Fo%2Fspaces%252FJUOUKxX4X7uzzyzY7fQb%252Fuploads%252F9JGUUwBv5bye38kS0TKq%252FEditorInputs.jpg%3Falt%3Dmedia%26token%3D5f57f10f-2ffb-4ce1-a7a7-ec6978a1d86b&width=768&dpr=4&quality=100&sign=57d08ae&sv=2)

The I/O system was designed in a way to be easily extensible. You can register custom events and functions with the I/O system for any of your actors using the `ActorIOInterface`. Here is a quick rundown on how you can expose custom events and functions:

1. Add the `ActorIOInterface` to your actor.
2. Override the `RegisterIOEvents` and `RegisterIOFunctions` of the I/O interface.
3. Use `RegisterIOEvent` to expose a C++ dynamic delegate, or blueprint event dispatcher.
4. Use `RegisterIOFunction` to expose a C++ UFUNCTION, blueprint function, or blueprint custom event.
5. After compiling and saving, your custom events and functions should now appear in the I/O editor for the actor.

# Logic Actors

Logic Actors are small "script-like" actors that help you create more complex level scripting logic. They usually only serve one specific purpose, so they can be used in any combination when working on levels. The plugin comes with a collection of logic actors such as Branch, Relay, Counter, Spawner, etc. All logic actors can be found in **Window → Place Actors**.

You can also create custom logic actors by subclassing the `LogicActorBase` class. The idea is that you create your own logic actors that interact with different parts of your game, such as adding/removing items from the player's inventory, updating the current objective, or starting an ambush that spawns enemies.

For more information visit: [Logic Actors](https://horizongames.gitbook.io/actorio/logic-actors)

![Logic Actors](https://horizongames.gitbook.io/~gitbook/image?url=https%3A%2F%2F1586816513-files.gitbook.io%2F%7E%2Ffiles%2Fv0%2Fb%2Fgitbook-x-prod.appspot.com%2Fo%2Fspaces%252FJUOUKxX4X7uzzyzY7fQb%252Fuploads%252Fk5eu7DgVGh4bDsHeuxyl%252FAbout.jpg%3Falt%3Dmedia%26token%3D0af4c6dc-5d2c-4ceb-8f34-fabb1f2c07e9&width=768&dpr=1&quality=100&sign=8635e146&sv=2)

# Video

Here is a quick video showcasing the plugin in action.

https://github.com/user-attachments/assets/02eb91c7-d2d8-4624-a207-c5be17bcff94

# Sample Project

A top-down shooter game is also available, showcasing how Actor I/O can be used for level scripting in Unreal Engine. See: [Actor I/O Shooter Sample](https://github.com/HorizonGamesRoland/ActorIOSample)

<img width="378" height="241" alt="screenshot_1" src="https://github.com/user-attachments/assets/7e7a87dc-8909-449d-924c-7b21bfec1ec2" /> <img width="378" height="241" alt="screenshot_2" src="https://github.com/user-attachments/assets/9db8ce28-7099-4ad2-ad81-f65bc2a272f4" />

# Why Use This?

You may be wondering why use this over blueprint interfaces? Aren't these the same thing?

While the system may look similar to regular interfaces, behind the scenes they are completely different. In fact, this is much closer to event bindings rather than interfaces. There is nothing wrong with using interfaces for hooking up game logic together, but it requires you to have a fairly robust system to be flexible enough. Again, not an issue just a different approach.

The biggest advantage of this approach is reduced memory usage, since you can avoid blueprint Casting with the I/O system. This can significantly lower memory usage, as the Cast node in Unreal causes the selected class to be loaded into memory with the blueprint. In larger projects, this can quickly spiral out of control if not managed properly. The I/O system can help you cut down on a lot of Cast nodes as the system uses the C++ reflection system to access members of a class, without casting to it.

# Debugging

The plugin is integrated with the [Map Check](https://dev.epicgames.com/documentation/en-us/unreal-engine/map-check-errors-in-unreal-engine) feature of Unreal in **Build → Map Check**. Use this to find build time errors, such as actions with invalid targets or function names.

![MapCheck](https://horizongames.gitbook.io/actorio/~gitbook/image?url=https%3A%2F%2F1586816513-files.gitbook.io%2F~%2Ffiles%2Fv0%2Fb%2Fgitbook-x-prod.appspot.com%2Fo%2Fspaces%252FJUOUKxX4X7uzzyzY7fQb%252Fuploads%252FgqdPP4kopw1d5EPT7epX%252FMapCheck.jpg%3Falt%3Dmedia%26token%3D1846ab29-cef8-47c3-8565-f1303004916f&width=768&dpr=4&quality=100&sign=9cb15b1b&sv=2)

All error messages and issues are logged to the [Output Log](https://dev.epicgames.com/documentation/en-us/unreal-engine/logging-in-unreal-engine) in **Window → Output Log**. Use the `-log` launch parameter to see logs when you are not playing in the editor (e.g. in Standalone Game play mode, or packaged builds).

Useful console commands to use:

- `ActorIO.DebugActions <bool>`: Enable I/O action execution messages. Enabled by default.
- `ActorIO.WarnAboutInvalidTarget <bool>`: Warn about missing or invalid target actor when executing I/O action.
- `ActorIO.LogNamedArgs <bool>`: Log named arguments to console when executing I/O action.
- `ActorIO.LogFinalCommand <bool>`: Log the final command sent to the target actor after executing I/O action.

# Help

- Documentation: [https://horizongames.gitbook.io/actorio](https://horizongames.gitbook.io/actorio)
- Discord: [Horizon Games](https://discord.gg/t8STNrGcU3)
- Email: horizongames.contact@gmail.com
