CONTENTS OF THIS FILE
---------------------

* Introduction
* Core features
* Requirements
* Installation
* Usage and configuration
  - Basic usage
  - Cascading exclusion
  - Opting out of the exclusion list
* How it works
* Frequently Asked Questions (FAQ)
* Maintainers


INTRODUCTION
------------

Views Exclusion List tries to solve a deceptively simple problem. You need to
exclude results of one view from another view, so neither of them have duplicated
results. That should be easy, right? Actually, there are a few contrib modules
aiming to do that. Here are some of them:

https://www.drupal.org/project/views_exclude_previous
https://www.drupal.org/project/other_view_filter
https://www.drupal.org/project/views_field_view
https://www.drupal.org/sandbox/dereine/1443726
https://www.drupal.org/sandbox/knoboid/2429901

So, why do we need another one? The number of modules trying to provide similar
functionality should give you a hint about how tricky this problem seems to be.
Over the years I've tried them all and none offered a proper solution: some have
operational limitations, and lack the flexibility needed for advanced use cases.
Others don't work reliably, and others simply don't work. So, whenever I needed
such functionality I always ended up using what I call the "global list hack"
(more on this below). And as in many other cases, the hack eventually evolved
into a full-fledged module.


CORE FEATURES
-------------

* Creates a Views filter to exclude results from other views.
* The filter configuration allows you to define exactly which views results
  will be exclude from the results of another view.
* Excluded views can (optionally) be loaded and executed if they have not been
  previously executed in the page in order to be able to identify results that
  should be excluded.


REQUIREMENTS
------------

- Views module.


INSTALLATION
------------

Install and enable this module as you would normally do with any other
contributed Drupal module. For further information, see:
https://drupal.org/documentation/install/modules-themes/modules-7


USAGE AND CONFIGURATION
-----------------------

BASIC USAGE

Let's say you have a page with three different Views blocks (A, B and C). All of
them list nodes of the same type, and you want to prevent nodes of either view A
or view B from being included (duplicated) in the view C.

I named the views in alphabetical order for a reason. When trying to exclude
results from another view, that view needs to be executed before the view trying
to exclude duplicated results, so we can know which results are duplicated in
first place. In other words, the order in which each view is located in the page
(and thus, executed) is also what defines which view is A, B or C. In our case,
view C should be located in the page after B, which should be located after A.

Take note that, while not a hard requirement (you'll have the option to overcome
this limitation by forcefully loading views that have not been executed yet) it
is strongly recommended that you plan your deduplication strategy according to
this principle to ensure maximum efficiency.

With that in mind:

1. Open the edit screen of the view C.
2. Under "Filter criteria", add a new filter of the type "Exclusion list" (under
   the Global category).
3. In the "Views/displays to exclude" field, select the displays of the views A
   and B that you are loading in the same page.
4. Optionally, check the "Force view loading" option. As mentioned before, this
   should be necessary only if the page doesn't load the views in the logical
   order, for example, if the view C is being loaded before the view A (this may
   happen if you're using Panels; check the FAQ for a better way to handle it).
   You also may need it if the view C loads additional results using Ajax.
   Generally speaking though, this option should be left unchecked unless you
   know you need it, in order to avoid loading the same view twice.
3. Save the changes and reload the page where your Views blocks are displayed.
   You shouldn't see any duplicates (results appearing in the view A or C) in
   the view C.

CASCADING EXCLUSION

OK cool. But what if I also need to exclude results of the view A from the view
B, and while we're at it, I also want to exclude results of C from a new view
called D that I just added to the page?

The filter will work just fine when doing cascading exclusion: A excluded from
B; A and B excluded from C; A, B and C excluded from D, and so on. In our case,
just repeat the steps above but this time applying the filter to the view B and
selecting the displays of the view A in the filter configuration, then repeat
again on view D adding view C displays. In this way, you can create very complex
relationships between multiple views ensuring that none of them will contain
duplicated records.

OPTING OUT OF THE EXCLUSION LIST

By default, every view that is executed during a page request is intercepted to
add its results to the global exclusion list. Under some circumstances however,
you may want specific views displays to be ignored, meaning that their results
would not be added to the list (perhaps because the result set is very large,
and you know you won't need to exclude these results from another view), or for
any other reason. In that case, you can opt-out specific views displays:

1. Open the edit screen of the corresponding view display.
2. Under the "Advanced area" locate the "Exclusion list" option (should be near
   to the end of the column). By default the link will read "Contributing
   results", which means that results of this display view are being accounted
   for by the exclusion list.
3. Click on the link and check the "Ignored by the exclusion list" box. This
   will prevent the results of this view display from being added to the list.
   Note that when a view display is being ignored by the exclusion list, their
   results will not be accounted for even if you add the same view display to
   the filter criteria of another view.
4. Click "Apply" and save the changes.


HOW IT WORKS
------------

As already mentioned, this module was born out of a "hack" I've been using for
years and has proven to be a simple yet effective and reliable method to
exclude other views' results. The basic idea is that when any view is executed,
a record of every result that is generated by this view is kept in memory.
We'll call this the "global exclusion list" and it stores every result generated
by any view that shares the same base table (ex., nodes). Then, when the filter
provided by this module is applied to a view, the view will use the contents of
the global exclusion list in order to identify possible results (duplicates)
that should be excluded from its own result set. Since the global exclusion list
stores only field id values (in nodes, the nid column) the whole process is very
performant and effective.

For years I tackled this task by simply throwing some hooks in a custom module
(hence its "hackish" origin). This worked fine for relatively simple scenarios,
but then I was comissioned to build a news site where the front page loads no
less than 20 different views, many of which need to exclude previous results in
cascade from multiple views displays. For my own sanity, I decided it was time
to write a proper module.


FREQUENTLY ASKED QUESTIONS (FAQ)
--------------------------------

* I'm adding views to my page using Panels, and after setting up the order of
  the panes accordingly, I still have to check the "Force view loading" option
  or previous results will not be excluded when they should.

  This issue is actually the result of how Panels renders the panes assigned to
  a handler (or "page variant"). For a number of factors (such as the region the
  pane is assigned to) Panels won't necessarily process panes in the order that
  they appear in the page. But fixing this is actually quite simple, just add a
  hook in a custom module that will process the contents of the array to ensure
  that they match the expected order:

  /**
   * Implements hook_panels_panes_prepared_alter().
   *
   * Change the order of the panes render array to follow the order defined in
   * the regions of the layout. Without this adjustment it may not be possible
   * to exclude nodes previously added to the global exclusion list.
   */
  function MY_MODULE_panels_panes_prepared_alter(&$vars, $context) {
    $region_weight = $context->plugins['layout']['regions'];
    $index = 0;
    foreach ($region_weight as $key => $value) {
      $region_weight[$key] = $index * 100;
      $index++;
    }
    foreach ($vars as $pid => $pane) {
      $vars[$pid]->render_weight = $region_weight[$pane->panel] + $pane->position;
    }
    uasort($vars, function ($item1, $item2) {
      return $item1->render_weight - $item2->render_weight;
    });
  }


MAINTAINERS
-----------

Current maintainers:

* Original Author: Proteo <https://www.drupal.org/user/2644289>
