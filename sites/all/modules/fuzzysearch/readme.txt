Fuzzy Search Module Read Me Project Home: http://drupal.org/project/fuzzysearch

=== Installation === - The following installation steps assume your drupal
installation is located at http://example.com with clean urls turned on.

To install this module simply:
1. Download the latest stable release from the
   projects page: http://drupal.org/project/fuzzysearch.
2. Unzip the package into your modules directory.
3. Go to your module administration page at http://example.com/admin/build/modules 
   and check to install the Fuzzy Search module.
4. Run the cron script on your site by visiting http://example.com/cron.php
5. You may want to run the cron a few times until it loads very fast, which 
   signifies that all of the index processing has taken place.
6. It is important to run cron at least once per day while using this module 
   because the content on your site will need to be reindexed as they are update
   and commented on.  Other modules may also queue your content for reindexing
   in order for them to operate properly as well.  To setup an automated cron
   job read the how to guide on drupal.org's handbook.

=== What is indexed? ===

Currently this module indexes all filtered node content, taxonomy terms
associated with the nodes, cck text fields associated with a node, comments left
on the node, and any text being returned by the call to hook_nodeapi with the
$op = 'update index'.

=== Fuzzy Search Settings ===

*Checking on the display scoring checkbox is helpful for debugging when you are
trying to fine tune score modifiers.  It will output completeness and score
values under each of the returned results.

*Setting a minimum completeness is helpful for ensuring that words are returned
only if a high enough percent matches. It is best to set this value 10 points
below your ideal minimum percentage. So if you wanted to match results with at
least 50% of the word matching, set this value to 40.  The setting applies to
each word that is in the index and not the phrase, this helps ensure that the
matches are relevant to the words in the phrase and not just all the letter
combinations in the phrase.

*Also note that when a phrase matches more than a single word the completeness
can be higher than 100%, this is because the completeness of each word is summed
and then sorted as a measure of accuracy in the result set.

* As of Drupal 6 you can filter results output by node type. This does not
affect search indexing, so you don't have to reindex if you change this.

* As of Drupal 6 you can choose the ngram length. This is the size of the chunks
words are broken into on indexing and searching. The default value is 3. The
lower the value, the more results (and more noise) you will get. Also, a lower
value will increase the size of your fuzzysearch_index table.

* As of Drupal 6 Fuzzy Search will try to highlight misspelled words. You can
set the accuracy by choosing a minimum spelling score from 0 to 100, where 100
means no misspellings are highlighted. You do not need to reindex when you
change this setting.

This provides a fuzzy spelling check by replacing bad (misspelled) ngrams with
a wildcard. It is possible to get false matches. For example, searching for
"rendition" will also highlight "condition" if your spelling score is low
enough. However, these kinds of matches are likely to have lower score
completeness and be sorted to the bottom of your results if your search term
exists in your content.

=== Fuzzy Search Block ===

The Drupal 6 version provides a block that performs a fuzzysearch on a query
in the path. This may be performance intensive, so use with caution.
If the fuzzysearch query is in the path, the block will return search matches of
node titles. It's up to you to put the query in your path like this:

http://example/node/add/question?fuzzysearch=arthritis%20knees%20pain

This would be good for similar content blocks, or to suggest existing content
before letting the user create new content.

=== Theming ===

As of Drupal 6, the module provides a template, fuzzysearch-result.tpl.php, that
you can copy to your theme folder and modify. This affects the search results
page. There are some theme functions you can override to theme the fuzzysearch
block, and you can also override block.tpl.php.

=== About Fuzzy Search ===

This module provides a fuzzy matching search engine for all nodes on a website.
Nodes are indexed when the sites cron job is run.  The module automatically
queues a node for indexing once it is submitted/updated or a comment has been
made on it.  Nodes can also be queued for reindexing by other modules when the
function fuzzysearch_reindex($nid, $module) is called.  Where $nid is the nid of
the node to have reindexed and $module is a string containing an identifier of
the module calling for the node to be reindexed.

Fuzzy matching is implemented by using qgrams.  Each word in a node is split
into 3 (default) letter lengths, so 'apple' gets indexed with 3 smaller strings 'app',
'ppl', 'ple'.  The effect of this is that so long as your search matches X
percentage (administerable in the admin settings) of the word the node will be
pulled up in the results.  One issue that is inherent with this method is cases
when a user searches for a word like 'athens' which contains the word 'the'
within it and has a completeness of 100%.  In order to account for this I have
implemented into the query that larger length words qgrams must match qgrams
from words with a similar length.  This is not a 100% solution but it does a
good job of keeping results closer in relevancy. Also, there is a list of common
stop words that are not indexed.


=== Contributed Modules Search Factor ===

This module also allows other contributed modules to modify the score of any
node being indexed.  Furthermore individual site administrators can then set
how important these modifiers are to their particular sites use.  Changing the
modifier score to 0 means that the modification being returned by that
particular module will have no effect on the scoring of the nodes on the site.
Setting the modifier to 10 means it will have maximum effect.

Example code of a contributed module implementing the scoring hook:

/** * Implementation of hook_search_score * @param $op 'settings' returns array
with information about the module (seen in the admin settings form) *  'index'
returns a score modifier to the node being indexed */

function fuzzysearch_search_score($op, $node) {
  switch ($op) {
    case 'settings':
    $info[] = array(
      'id' => 'fuzzysearch',
      'title' => 'Fuzzysearch',
      'description' => 'This score factor depends on how fuzzy something is', );

      return $info;
    case 'index':
      // do some processing to figure out what score to bring back
      // the node variable is available for this processing $score = 5; 
      // setup an array with matching title in the scoring 'op' so that proper
      // score multipliers can be set for this score modifier
      $scores[] = array(
        'id' => 'fuzzysearch',
        'score' => $score,
      );
      return $scores;
  }
}

=== Contributed Stop Words Filtering ===

I have inserted a hook_search_filter($text) that will allow any module to tap
into the words just before indexing, this would allow anyone to
insert/remove/modify words based on text already about to be indexed.  The $text
variable is passed by reference so all changes need only be made to the $text
variable.

There is also a set of English stopwords hard coded into the module. @todo move
this into admin settings or a file.

=== About The Author ===

This module was created for Drupal as part of Google Summer of Code 2007.

Blake Lucchesi www.boldsource.com blake@boldsource.com

Drupal 6 version maintained by awolfey.