package MLocarna::PairwiseAligners;


use MLocarna;

############################################################
##
## Locarnate supports multiple Pairwise alignment tools
## all of which are defined here
##
############################################################

sub locarna_compute_pairwise_alignments {
    
  my ($options, $results_path, $sequences, $bindir) = @_;

  my $numSequences = scalar(@{$sequences});

  # get locarna parameters
  my $parameter = $options->{pairwise_aligner_parameter} or "";

  my $counter = 0;
  # how many pairwise alignments must be calculated?
  my $number = $numSequences * 
      ($numSequences - 1) / 2;

  for (my $i = 0; $i < $numSequences - 1; ++$i) {
    # the path to the dotplot of the this sequence
    my $first = $results_path.'/dp/S'.($i + 1) .= '_dp.ps';

    # iterate over all other sequences in the rest of the array and compute
    # pairwise alignment
    for (my $j = $i + 1; $j < $numSequences; ++$j) {
      ++$counter;
      *STDERR->print("\r".(' 'x80));
      *STDERR->print("\r".'Calculate pairwise alignment '.
                     $counter.'/'.$number.' (Seq '.($i + 1).' <-> Seq '.
                     ($j + 1).')...');
      # path to the dot plot of the other sequence
      my $second = $results_path.'/dp/S'.($j + 1) . '_dp.ps';

      # path to the output file
      my $result = $results_path.'/pair/S'.($i + 1).'_S'.($j + 1).'.aln';

      # assemble the call to locarna
      my $call = $bindir .'/locarna '.$parameter.
          ' --clustal='.$result.' --write-structure '
          .$first.' '.$second.' 1>/dev/null 2>/dev/null';

      print "$call\n";
      my $code = system($call);
      if ($code) {
        print('Can\'t perform locARNA calculation'.
                            " (developed under locarna 1.4.5, call:\n".$call.')');
      }
    }
  }
  
  *STDERR->print(' done!'."\n");

  *STDERR->print('parse pairwise alignments...'."\r");

  my @alignments = ();

  for (my $i = 0; $i < $numSequences - 1; ++$i) {
    for (my $j = $i + 1; $j < $numSequences; ++$j) {
      my $file_name = $results_path.'/pair/S'.($i + 1).
          '_S'.($j + 1).'.aln';
          
      my ($header, $aln) = MLocarna::read_clustalw_aln($file_name);
      my $score;
      if ($header =~ m/Score: ([\d|-]+)/) {
        $score = $1;
      }
      push(@alignments, {"rows" => $aln, "score" => $score});
    }
  }

  *STDERR->print('Parse pairwise alignments... done!'."\n");

  return(\@alignments);
}

sub locarna_parse_pairwise_alignments {

}

1;
