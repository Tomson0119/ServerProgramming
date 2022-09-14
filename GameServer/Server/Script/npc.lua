myid = 99999
count = 0

attack_msg = 'Take this!'
hurt_msg = 'Ahh'

function set_uid(x)
   myid = x
end

function event_npc_attack(player_id)
   API_SendMessage(myid, player_id, attack_msg)
end

function event_npc_hurt(player_id)
   API_SendMessage(myid, player_id, hurt_msg)
end
