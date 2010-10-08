Purpose:

Think about you can fetch the projects and all the meta informations like releases (and there download URL!) and all other fields using a central API
And think about you can do this from different palces, drupal.org with project, your own feature server under "yourdomain.org" and what ever you like.
All you need is a update-feed integration like the one from Project or Fserver, or your write your own fetch handler fetching those informations out of .. hmm, git, some DB, a FILE?

Scope:
That module should be for developers which want to work with the Project informations of Drupal modules/themes/whatever for building anykind of tools.

Usecase:
-Auto Update-
Like Drupal module for auto-updating your site. You fetch the current versions available and decide buy GUI what you want to update. The module downloads the module and does the rest.

-Drush module / version autocompletition-
Well yeah, think about you get a autocompletiton on modules / version / api on the console. Lovely!

-Tell me!-

The initial idea for this came from the http:/drupal.org/project/project_api which does generaly the same, but lacks things like multi-server support or the fetching of releases (yet).