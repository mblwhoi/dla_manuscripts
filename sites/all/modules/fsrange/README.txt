$Id: README.txt,v 1.1.2.1 2009/09/09 13:21:28 zyxware Exp $

PURPOSE
-------

This module lets you define 'range facets'. Range facets allow your
users to easily find nodes whose fields fall within certain ranges of
values. For example, if you have a numeric field holding the price of a
product, you can define a 'Price' facet consisting of three ranges of
values (for cheap products, medium priced products, and expensive
products). Once range facets are defined, you can, like with any other
facet, add them to your search environment.

Note that this module supports only fixed ranges --which the
administrator defines. It _doesn't_ provide text-fields, or any other
means of input, by which the user may specify his own desired range.

INSTALLATION
------------

After extracting this package on the server, enable the following two
modules:

  * Faceted Search Ranges
  * Faceted Search Ranges Administration

And any or all of the following modules:

  * Numeric Range Facets
  * Textual Range Facets
  * Taxonomy Range Facets
  * Date Range Facets

USAGE
-----

On your faceted search settings page (Administer > Site configuration > 
Faceted Search) you'll now find a new tab, "Range facets". Visit it. On a
fresh installation no range facets are defined, and that page would say so.
Start by clicking the "Define a new range facet" link, and follow the
instructions. Once you define one or more range facets you'll want to add
them to your search environment(s).

RANGES
------

Associated with every range facet are ranges of values. When you're
setting up a range facet you're provided with a textarea into which you
are to type ranges of values to display, as categories, to the user.

Ranges have a certain syntax you must follow. Let's define the ranges
for our 'Price' facet:

    0..10        | Cheap ($0 to $10)
      0..1]      | Dirt cheap (1 dollar or less)
    10..80       | Medium ($10 to $80)
    80..inf      | Expensive ($80 and up)
      1000..inf  | Really expensive ($1000 and up)
      10000..inf | Really really expensive ($10,000 and up)

A range is defined by two endpoints, separated by two dots (".."). An
optional label, to display to the user, follows a pipe character (You
_don't_ need to align the labels in a nice column as I did above.)

Ranges may be nested. The nesting is determined by the amount of space
at the beginning of the line.

RANGE BOUNDARIES
----------------

A range endpoint is either 'closed' (included in the range) or 'open'
(excluded from the range). The notation to use is the customary one, of
round or square brackets. For example, in the range "(3..6]" the value 3
is excluded and the value 6 is included. In other words, this range is
{x | 3 < x <= 6}. If no brackets are explicitly used, the default is to
include the first endpoint and to exclude the second one (that is,
"0..10" is equal to "[0..10)").

There are two special endpoints: "inf" and "-inf". These stand for
infinity and minus infinity, respectively.

TEXTUAL RANGES
--------------

Textual ranges are just like numeric ones, but you use letters, or
strings, instead of numbers. Here's how to partition our node titles
into four ranges:

  a..f | A to E
  f..j | F to I
  j..zzz | J to Z
  -inf..inf | All others

Note that the range "a..f" doesn't matches values beginning with "f": we
explained earlier that by default the second endpoint is 'open'. Note
that changing it to "a..f]" won't make it match values beginning with
"f" either, because --for example-- the word "fast" is greater,
lexicographically, than a mere "f".

Admittedly, the "zzz" trick isn't an elegant one (you should be able to
replace this string by "{", as it follows "z" in the Unicode table, but
it's a detail which is hard to remember).

The "-inf..inf" range is a catchall that would match any (non-NULL)
value not matched by previous ranges. So in our case it would match
values beginning with digits, punctuation, or non-English letters (or
values beginning with three "z").

DATE RANGES
-----------

Dates are to be typed in the 'ISO format' only: YYYY-MM-DD HH:SS, where
the right-most parts, till the year, can be omitted. Examples:

  -inf..2002 | Publications till 2002
  2002..inf  | Publications from 2002 

A special value is "now", which stands for the current time. It may be
followed by a plus or minus sign and then a number of seconds. But it
gets even more useful: you can type an arithmetic expression involving
numbers and the multiplication operator:

  now - 60*60*24*7 .. inf   | During the past week
    now - 60*60*24*1 .. inf | During the last 24 hours
  now - 60*60*24*30 .. inf  | During the past month
  now - 60*60*24*365 .. inf | During the past year

HOW THIS MODULE WORKS
---------------------

This module works by translating the range definitions to an SQL
expression. For example, the 'Price' facet would get translated into:

  CASE
    WHEN field_price_value >= 0  AND field_price_value < 10 THEN 'R-00000'
    WHEN field_price_value >= 10 AND field_price_value < 80 THEN 'R-00001'
    WHEN field_price_value >= 80 THEN 'R-00002'
    ELSE NULL
  END

OVERLAPPING RANGES
------------------

By examining the SQL expression above we can understand why the "-inf ..
inf" catchall range, demonstrated in the "textual ranges" section,
serves its purpose. However, examination leads us to suspect that
"During the past month" will fail, because its predecessor, "During the
past week", would eat up its values. This problem occurs only when
presenting to the user, in brackets, the number of nodes found (as in
"During the past month (53)"). To fix this the module can add to a
range's node count the sum of all previous ranges' counts. A checkbox on
the settings page governs this behavior. Note that you must use the
default "Range ID, asc" sort for this feature to work correctly.

USING PHP
---------

It's possible to specify ranges using PHP. Example:

  if ($levels) return array();
  return array(
    305 => array('from' => 1,  'to' => 10, 'label' => t('Cheap')),
    306 => array('from' => 10, 'to' => 80, 'label' => t('Medium'),
                  'boundaries' => array('from' => 'closed', 'to' => 'open')),
    307 => array('from' => 80, 'to' => 'inf', 'label'=> t('Expensive')),
  );

Your code should return an array of ranges for the specific level
described in $levels. Since our ranges, in this example, aren't nested,
we return an empty array for any level which isn't the top-level one.

This $levels mechanism allows us to define huge --and possibly
infinite-- hierarchies without consuming precious memory. That's because
we're returning the ranges in a certain level only, and not the ranges
in the entire hierarchy.

The '305', '306', '307' keys aren't important: they're there to show
that you can control the "range ID" (described in API.txt).

