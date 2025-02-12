[![GitHub Repo stars](https://img.shields.io/github/stars/HorizonGamesRoland/ActorIO?style=plastic&color=yellow&logo=github)](https://github.com/HorizonGamesRoland/ActorIO/stargazers) [![GitHub License](https://img.shields.io/github/license/HorizonGamesRoland/ActorIO?style=plastic&logo=apache)](https://www.apache.org/licenses/LICENSE-2.0) [![Discord](https://img.shields.io/discord/1337396883366219808?style=plastic&logo=discord&logoColor=white&label=discord&color=%235865F2)](https://discord.gg/t8STNrGcU3) [![Static Badge](https://img.shields.io/badge/Documentation-blue?style=plastic&logo=gitbook&logoColor=white)](https://horizongames.gitbook.io/actorio)


# About

Actor I/O is a level scripting tool for Unreal Engine, featuring an input/output system similar to the **Source Engine** by Valve™ Software. At its heart, the plugin is an event binding system where actions are executed in reaction to an event being triggered. Using the C++ reflection system of Unreal Engine, the system can react to any dynamic delegate be it C++ or blueprint, and essentially call any function by name with parameters.

> [!IMPORTANT]
> Please note that the plugin is currently in **Beta**! It should be stable enough to be used in projects, but certain issues or bugs can still exist. Please make sure to use version control before adding it to your project, or test it in a separate project.

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

Logic Actors are small "script-like" actors that only server one specific purpose. They usually only do one small task, so they can easily be combined when scripting levels. The plugin comes with a collection of Logic Actors that you can use for level scripting. You can find all of them in the **Place Actors** tab of the engine. You can also create custom logic actors by subclassing the `LogicActorBase` class.

![Logic Actors](https://horizongames.gitbook.io/~gitbook/image?url=https%3A%2F%2F1586816513-files.gitbook.io%2F%7E%2Ffiles%2Fv0%2Fb%2Fgitbook-x-prod.appspot.com%2Fo%2Fspaces%252FJUOUKxX4X7uzzyzY7fQb%252Fuploads%252Fk5eu7DgVGh4bDsHeuxyl%252FAbout.jpg%3Falt%3Dmedia%26token%3D0af4c6dc-5d2c-4ceb-8f34-fabb1f2c07e9&width=768&dpr=1&quality=100&sign=8635e146&sv=2)

# Why Use This?

You may be wondering why use this over blueprint interfaces? Aren't these the same thing?

While the system may look similar to regular interfaces, behind the scenes they are completely different. In fact, this is much closer to event bindings rather than interfaces. There is nothing wrong with using interfaces for hooking up game logic together, but it requires you to have a fairly robust system to be flexible enough. Again, not an issue just a different approach.

The biggest advantage of this approach is reduced memory usage, since you can avoid blueprint Casting with the I/O system. This can significantly lower memory usage, as the Cast node in Unreal causes the selected class to be loaded into memory with the blueprint. In larger projects, this can quickly spiral out of control if not managed properly. The I/O system can help you cut down on a lot of Cast nodes as the system uses the C++ reflection system to access members of a class, without casting to it.

# Help

- Documentation: [https://horizongames.gitbook.io/actorio](https://horizongames.gitbook.io/actorio)
- Discord: [Horizon Games](https://discord.gg/t8STNrGcU3)
- Email: horizongames.contact@gmail.com
