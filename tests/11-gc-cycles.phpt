--TEST--
Check for get_gc property handler
--FILE--
<?php
 
class x {
        public $t = null;
 
        public function __construct() {
                $this->t = Event::timer(new EventBase(), function () { });
                $this->t->free();
        }
}
 

echo "1";
new x();
gc_collect_cycles();
echo "2"; // we had segfault here
?>
--EXPECT--
12
