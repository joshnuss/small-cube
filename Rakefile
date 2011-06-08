HEX_PATH = "Debug/small_matrix.hex"
PART = 'm48'
PROGRAMMER = 'usbtiny'

desc 'compile program'
task :compile do
  system "avr-gcc -g -Os -Wall -mcall-prologues -mmcu=atmega8 small_matrix.c"
end

namespace :burn do

  def avrdude(cmd)
    system "avrdude -c #{PROGRAMMER} -p #{PART} #{cmd} -F"
  end

  desc 'burn flash with program'
  task :flash => :compile do
    avrdude "-U flash:w:#{HEX_PATH}"
  end

  desc 'burn fuses with settings'
  task :fuses do
    avrdude "-U lfuse:w:0x42:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m"
  end

end
