/***************************************************************************
                          plugin_loader.cpp  -  description
                             -------------------
    begin                : Mon Dec 22 2003
    copyright            : (C) 2003 by Gene Ruebsamen
    email                : gene@erachampion.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "plugin_loader.h"

// our global factory for making shapes
map<string, maker_t *, less<string> > factory;

// Singleton pattern to enforce single instance of this class
plugin_loader* plugin_loader::_instance = 0;

plugin_loader* plugin_loader::Instance(StatusPort *ptr)
{
	if(_instance == 0)
	{
		_instance = new plugin_loader();
		_instance->status_ptr = ptr;
	}
	return _instance;
}

plugin_loader::plugin_loader()
{
	FILE *dl;   // handle to read directory
  char *command_str = "ls ~/.liarliar/plugins/*.so";  // command string to get dynamic lib names
  char in_buf[BUF_SIZE]; // input buffer for lib names

	//status_ptr = ptr;
	// get names of all dynamic libraries in the "plugins" dir
	dl = popen(command_str, "r");
	if (!dl)
	{
		perror("popen");
		exit(-1);
	}

	void *dlib;

	#ifdef _DEBUG
	cout << "Loading Plugins:" << endl;
	#endif

	while (fgets(in_buf, BUF_SIZE, dl))
	{
		// trim the whitespace
		char *ws = strpbrk(in_buf, " \t\n");
		if (ws)
			*ws = '\0';
		
		#ifdef _DEBUG
		cout << in_buf << endl;
		#endif
		dlib = dlopen(in_buf, RTLD_NOW);
		if(dlib == NULL)
		{
			cerr << dlerror() << endl;
			exit(-1);
		}

		// add the handle to the list
		dl_list.insert(dl_list.end(), dlib);
	}

	int i=0;
	// create an array of plugin names
	for (fitr=factory.begin(); fitr!=factory.end();fitr++)
	{
		plugin_names.insert(plugin_names.end(),fitr->first);
		// now create the plugins themselves by inserting them on the plugin list
		plugin_list.insert(plugin_list.end(),factory[plugin_names[i]]());
		i++;
	}	
}

plugin_loader::~plugin_loader()
{
	// destroy any plugin's we've created
	for (plugin_list_itr=plugin_list.begin();plugin_list_itr!=plugin_list.end();plugin_list_itr++)
	{
		delete *plugin_list_itr;
	}

	// close all dynamic libs we've opened
	for (dl_list_itr=dl_list.begin();dl_list_itr!=dl_list.end();dl_list_itr++)
	{
		dlclose(*dl_list_itr);
	}
}

// Return a pointer to the user selected plugin.  1/23/03 <gene> - fixed to correctly use the
// factory map.  If user has not yet selected a plugin, the default first plugin will be returned.
plugin* plugin_loader::execute(string name)
{
	plugin *temp_ptr;

	if (name == "")  // default if no plugin has been selected
	{
		temp_ptr = *(plugin_list.begin());
		assert(plugin_list.size() > 0);
	}
	else             // otherwise use selected plugin
	{
		temp_ptr = factory[name]();
	}

	// provide the selected plugin with a pointer to the status window for GUI updates
	temp_ptr->status_ptr = status_ptr;
	return temp_ptr;
}

vector<string> plugin_loader::get_names()
{
	return plugin_names;
}
