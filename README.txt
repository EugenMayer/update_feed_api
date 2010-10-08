Purpose:

Think about you can fetch the projects and all the meta informations like releases (and there download URL!) and all other fields using a central API
And think about you can do this from different palces, drupal.org with project, your own feature server under "yourdomain.org" and what ever you like.
All you need is a update-feed integration like the one from Project or Fserver, or your write your own fetch handler fetching those informations out of .. hmm, git, some DB, a FILE?

As i described before, you can handle several sources. That means there is no issue having views on your own feature server and on drupal.org. Both projects are saved and can be selected by there source. Easy.

As iam using dbtng this API should be compatible to D6 and D7.

Scope:
That module should be for developers which want to work with the Project informations of Drupal modules/themes/whatever for building anykind of tools.

Usecase:
-Auto Update-
Like Drupal module for auto-updating your site. You fetch the current versions available and decide buy GUI what you want to update. The module downloads the module and does the rest.

-Drush module / version autocompletition-
Well yeah, think about you get a autocompletiton on modules / version / api on the console. Lovely!

-Tell me!-

The initial idea for this came from the http:/drupal.org/project/project_api which does generaly the same, but lacks things like multi-server support or the fetching of releases (yet).


Workflow:
Update feed entries are featch (project list) and for each project, the project informations and releases are fetched. Those informations are stored in the local database for usage with anything you like.
Use it for autocompletition or what ever you like. The database will be regenerated using cron jobs and caches.


API:
project.class.inc
  This is a abstraction of a Project. You can load/save/update a project from a Database or from a feed. You will use this container for your purpose like autocompletition.
  It also has an interface to work with the project release (of the current project identified by the server_key, short_name, api version).
  You can extend this class with needed methods for your applications or change some of its behavior
  
release.class.inc
  Container for a release with its meta data. You can load/save/update a release using this class (or the factory). Exactly the same interface as a release.
  
update_feed.class.inc
  A basic implementation of a update feed fetcher for drupalas "Project" module (like its used on drupal.org) and for the feature server. You can change the way sources are fetched or from where.
  Paths on the server are abstracted with tokens, so you can override and redefine you own protocols with only some few lines of code
  
factories.class.inc
  Just some factories to load Projects / Releases
  
feedentrycontainer.class.inc
  Thats actually the basic abstract class of a Project / Release handling all the shared tasks like saving / loading and the data storage (getter, setter).

