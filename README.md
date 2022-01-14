## Subsystem Browser Plugin for Unreal Engine

Subsystems are great, they provide easy way to extend engine functionality and implement new features!
Check [Programming Subsystems](https://docs.unrealengine.com/4.27/en-US/ProgrammingAndScripting/Subsystems/) article if you haven't tried them yet.

But there is nothing in Unreal Engine Editor that could be used to visualize them or allow to change their property values in runtime.

The Subsystem Browser Plugin comes to rescue with a dedicated panel to display which subsystems currently active in world and enables viewing and editing of their properties.

## Key Features

 * List of all avaiable subsystem types with search bar and customizable filters
   * Engine Subsystems
   * Editor Subsystems
   * Game Instance Subsystems
   * World Subsystems
   * Local Player Subsystems
 * Ability to view subsystems in different worlds 
 * Automatic toggle between PlayInEditor and Editor worlds when Starting / Stopping PIE
 * Ability to easily view and modify subsystem properties in Details View
 * Ability to easily identify which subsystems came from which module and if that module is a game module
 * Quick navigation to subsystem source file via "Open Source File" context menu
 * Quick actions to export properties as ini config sections via "Export Config" context menu
 * Browser state (chosen filters, category folders state) is saved to local editor settings

## Unreal Engine Versions

Tested with 4.25-4.27, UE5EA, UE5-main and can be easily modified to work with older engine versions.

## Contributing

Please report any issues with GitHub Issues page for this repository.

If you want to suggest changes, improvements or updates to the plugin use GitHub Pull Requests.

## License

SubsystemBrowserPlugin is available under the MIT license. See the LICENSE file for more info.

![](Images/UE4-Preview.png)
![](Images/UE4-Search.png)
![](Images/UE4-ContextMenu.png)
![](Images/UE4-ViewOptions.png)

![](Images/UE5-Preview.png)

---

Special Thanks to Unreal Slackers Community Discord and #cpp 
