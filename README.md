# About

Actor I/O is a level scripting tool for Unreal Engine, featuring an input/output system similar to the **Source Engine** by Valve™ Software. At its heart, the plugin is an event binding system where actions are executed in reaction to an event being triggered. Using the C++ reflection system of Unreal Engine, the system can react to any dynamic delegate be it C++ or blueprint, and essentially call any function by name with parameters.

> [!IMPORTANT]
> Please note that the plugin is currently in **Beta**! It should be stable enough to be used in projects, but certain issues or bugs can still exist. Please make sure to use version control before adding it to your project, or test it in a separate project.

# Usage

In order to use the I/O system you need to add `ActorIOActions` to actors. Actions are the driving force behind the whole system. Each action represents an event binding, which calls the designated function when that event is triggered. Actions exist in the game world as objects, with their lifetime tied to the actor - more specifically the I/O component of the actor.

Go to **Windows → Actor I/O** to open the action editor tab. From here you can add or remove actions to an actor. The editor is broken up into two tabs: Outputs and Inputs. The **Outputs** tab is your main editing environment, it shows all actions that are tied to this actor. You can add new actions to the actor with the new action button. The **Inputs** tab shows all actions that are targeting this actor. This tab is purely for debugging and overview purposes. You cannot edit or change actions from here, only from the calling actor.

Here is a quick rundown on how you can expose custom events and functions to the I/O system:

1. Add the `ActorIOInterface` interface to your actor.
2. Override the `RegisterIOEvents` and `RegisterIOFunctions` of the I/O interface.
3. Use `RegisterIOEvent` to expose a C++ dynamic delegate, or blueprint event dispatcher.
4. Use `RegisterIOFunction` to expose a C++ UFUNCTION, blueprint function, or blueprint custom event.
5. After compiling and saving, your custom events and functions should now appear in the I/O editor for the actor.

More detailed documentation will be available soon.

# Why Use This?

You may be wondering why use this over blueprint interfaces? Aren't these the same thing?

While the system may look similar to regular interfaces, behind the scenes they are completely different. In fact, this is much closer to event binding rather than interfaces. There is nothing wrong with using interfaces for hooking up game logic together, but it requires you to have a fairly robust system to be flexible enough. Again, not an issue just a different approach.

The biggest advantage of this approach is reduced memory usage, since you can avoid blueprint Casting with the I/O system. This can significantly lower memory usage, as the Cast node in Unreal causes the selected class to be loaded into memory with the blueprint. In larger projects, this can quickly spiral out of control if not managed properly. The I/O system can help you cut down on a lot of Cast nodes as the system uses the C++ reflection system to access members of a class, without casting to it.
