package MLocarna::Trees;

############################################################
##
## package for constructing trees by PGMA
## and handling tress in NEWICK format
##
############################################################


use 5.008003;
use strict;
use warnings;

use Text::ParseWords; ## for tokenizing

require Exporter;
    
# set the version for version checking
our $VERSION     = 1.00;

our @ISA         = qw(Exporter);
our @EXPORT      = qw(
upgma_tree
newick_tree_to_postorder
tree_partitions
check_tree_labels
project_tree
);

our %EXPORT_TAGS = ();

# your exported package globals go here,
# as well as any optionally exported functions
our @EXPORT_OK   = qw(
$node_sym
);

our $node_sym="\$\$nodesym";


########################################
## upgma_tree( $names ref to list, $score_matrix ref to matrix of scores)
##
## compute a upgma-tree
##
## $names ref to a list of names
## $score_matrix ref to a matrix (array of arrays) of scores
## 
## the output uses the names in list @$names. Consequently, names
## should be unique and indices in $@names and matrix @$score_matrix
## should correspond
##
## returns tree as string in NEWICK tree format
##
########################################
sub upgma_tree : prototype($$) {
    # print "Compute UPGMA-Tree ...\n";
    
    my ($names, $score_matrix) = @_;    
    
    ## compute tree by upgma applied to score matrix
    
    my @clusters; # a list of the clusters
    my @trees; # a list of sub-trees
    my @cluster_sizes;
    
    for (my $i=0; $i<@$names; $i++) {
	$clusters[$i]=$i;
	$trees[$i]=quotemeta($names->[$i]);
	$cluster_sizes[$i]=1;
    }
        
    my $NEG_INFINITY=-1e10;
    
    while ($#clusters>0) {
	
	#print "@clusters\n";
	#print "@trees\n";
	#print "@cluster_sizes\n";
	
        ## find the nearest two clusters
	
	my $max_i;
	my $max_j;
	my $max_score=$NEG_INFINITY;
	for (my $i=0; $i<=$#clusters; $i++) {
	    for (my $j=$i+1; $j<=$#clusters; $j++) {
		my $score=$score_matrix->[$clusters[$i]][$clusters[$j]];
		if ($score > $max_score) {
		    $max_i=$i; 
		    $max_j=$j;
		    $max_score=$score;
		}
	    }
	}
	
	
	## recompute distances
	my $cluster_i = $clusters[$max_i];
	my $cluster_j = $clusters[$max_j];

	$clusters[$max_j] = $clusters[$#clusters];
	$clusters[$max_i] = $clusters[0];
	$clusters[0] = $cluster_i;
	
	for (my $i=1; $i<$#clusters; $i++) {
	    $score_matrix->[$clusters[0]][$clusters[$i]]
		= 
		($cluster_sizes[$cluster_i] * $score_matrix->[$cluster_i][$clusters[$i]]
		 + $cluster_sizes[$cluster_j] * $score_matrix->[$cluster_j][$clusters[$i]])
		/ ($cluster_sizes[$cluster_i]+$cluster_sizes[$cluster_j]);
	    $score_matrix->[$clusters[$i]][$clusters[0]] = 
		$score_matrix->[$clusters[0]][$clusters[$i]];
	}
	
	my $new_tree="($trees[$cluster_i],$trees[$cluster_j])";
	
	$trees[$clusters[0]]=$new_tree;
	
	#$trees[$clusters[$#clusters]]="undef";
	#$cluster_sizes[$clusters[$#clusters]]=0;
    
	$cluster_sizes[$clusters[0]]=$cluster_sizes[$cluster_i]+$cluster_sizes[$cluster_j]; 
	
	$#clusters--;
    }


    return $trees[$clusters[0]].";";
}

########################################ä
## tree_partitions($tree_postorder ref of list)
##
## generate partitions out of the postorder tree
##
## $tree_postorder ref of list representing tree in postorder (as
## generated by newick_tree_to_postorder)
##
## returns ref of list of refs of partitions of leaves due to the tree; a
## partition is represented as a list/subset of leaves
##
########################################
sub tree_partitions: prototype($) {
    my $tree_postorder = shift;

    my @result;
    
    my @stack;
    
    for my $item (@$tree_postorder) {
	if ($item eq $node_sym) {
	    my @op1 = @{ $stack[-2] };
	    my @op2 = @{ $stack[-1] };
	    
	    $#stack-=2;
	    	    
	    my @op12 = (@op1, @op2);
	    
	    push @stack, \@op12;
	} else {
	    push @stack, [ $item ];
	}
	push @result, $stack[-1];
    }
    $#result-=2; # the last is empty, the one before symmetric
    return \@result;
}

########################################
##
## unquote a string using Text::ParseWords
##
## param string
## return unquoted string
##
sub unquote {
    my ($s)=@_;
    my @strings=parse_line(" ", 0, $s);
    return join " ",@strings;
}

########################################ä
## newick_tree_to_postorder($tree string)
##
## Translates a newick tree format string into a list of nodes in postorder
##
## $tree string in newick tree format 
##
## returns ref to list of nodes/leaves in postorder, use $node_sym for inner nodes
##
## supports quotation via module Text::ParseWords
##
## dies if tree is not parsable
##
########################################
sub newick_tree_to_postorder {
    my ($tree) = @_;

    $tree =~ s/;$//; # allow that the tree string is terminated by ';'
    
    my @tokens = parse_line('[,()\s]', "delimiters", $tree);
    
    my @list=();
    
    my $brcount=0;
    for (my $i=0; $i<@tokens; $i++) {
	my $tok=$tokens[$i];
	
	next if !defined($tok);
	
	if ($tok eq "(") {
	    $brcount++;
	} elsif ($tok eq ")") {
	    $brcount--;
	    if ($brcount<0) {
		die "Parse error in tree.";
	    }
	    
	    push @list, $node_sym;
	} elsif ($tok eq ",") {
	    ## ignore, although we could do syntax checking
	} elsif ($tok =~ /^\s*$/)  {
	    ## ignore whitespace
	} else {
	    push @list, unquote($tok);
	}
    }

    return \@list;
}

########################################
## old version of newick tree parsing without quoting support
sub newick_tree_to_postorder_old {
    my ($tree) = @_;

    my @list;
    
    #$tree =~ s/:[\d.e-]+//g;
    
    $tree =~ s/;$//; # allow that the tree string is terminated by ';'

    my $brcount=0;
    for (my $i=0; $i< length $tree; $i++) {
	my $c=substr $tree,$i,1;
	
	if ($c eq "(") {
	    $brcount++;
	} elsif ($c eq ")") {
	    $brcount--;
	    if ($brcount<0) {
		die "Parse error in tree.";
	    }
	    
	    push @list, $node_sym;
	} elsif ($c eq ",") {
	    ## ignore, although we could do syntax checking
	} else {
	    my $rest=substr $tree,$i;
	    $rest =~ /^([^(),]+)/;
	    my $token = $1;
	    $i+=(length $token)-1;
	    push @list, $token;
	}
    }

    return \@list;
}


########################################
## check whether a tree contains a set of labels
##
## @param $tree ref to tree in postorder 
## @param labels ref to list of labels
sub check_tree_labels {
    my ($tree,$labels) = @_;
    
    my %existing_labels;
    
    foreach my $i (@$tree) {
	$existing_labels{$i}=1;
    }
    
    foreach my $i (@$labels) {
	if (!exists $existing_labels{$i}) {
	    return 0;
	}
    }

    return 1;
}

########################################
## project a tree to a set of labels
##
## @param $tree ref to tree in postorder 
## @param labels ref to list of labels
## 
## @returns tree in postorder that is a sub-tree of @$tree
## and contains exactly the labels @$labels

sub project_tree {
    my ($tree,$labels) = @_;
    
    my @stack;

    foreach my $item (@$tree) {
	my @list=();
	
	if ($item eq $node_sym) {
	    my @x = @{ $stack[-2] };
	    my @y = @{ $stack[-1] };
	    $#stack-=2;
	    
	    if (@x==0 && @y==0) {
		@list = ();
	    } elsif (@x==0) {
		@list = @y;
	    } elsif (@y==0) {
		@list = @x;
	    } else {
		@list=(@x,@y,$node_sym);
	    }
	} else {
	    @list=();
	    if (grep (/^$item$/, @$labels)!=0) {
		@list=($item);
	    }
	}
	push @stack, [ @list ];
    }
    
    return $stack[0];
}
