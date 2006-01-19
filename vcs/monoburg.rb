class Vcs

  # See http://rubyforge.org/projects/vcs
  # and http://vcs.rubyforge.org

  protocol_version '0.1'
  
  def monoburg_commit! ( *args )
    common_commit!("monoburg <%= rev %>: <%= title %>", *args) do |subject|
      mail!(:to => %w[tiger-patches@lrde.epita.fr], :subject => subject)
    end
  end
  alias_command :mbci, :monoburg_commit
  default_commit :monoburg_commit

end # class Vcs
