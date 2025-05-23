#pragma once

/**
 * Configuration menu module. It is responsible for providing reusable
 * functionality for defining menu-like user interfaces. A menu is always a
 * vertical list of 'selector bars'. A selector bar is a horizontal rectangle
 * that has a name of the configuration value that is being controlled and
 * a cell value that is being controlled by the user input.
 *
 * The design is that the user is supposed to navigate between the configuration
 * options by pushing up/down and then toggle between respective configuration
 * values by pushing left/right. This is similar to how configuration menus work
 * on simple systems such as monitors or TV sets.
 */

// TODO: think about exactly what functionality is to be included in this module
// there are two options:
// - either expose a single function that allows for drawing generic config menus
// given a configuration struct (the menu would be rendered dynamically based
// on the shape of the struct)
// - or provide a toolkit similar to UI libraries allowing for rendering of
// configuration bars, confirmation buttons and other things.
