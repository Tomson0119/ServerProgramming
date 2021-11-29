myid = 99999;
count = 0;

function set_uid(x)
   myid = x;
end

function event_player_move(player, dir, msg)
   player_x = API_get_x(player);
   player_y = API_get_y(player);
   my_x = API_get_x(myid);
   my_y = API_get_y(myid);
   if (player_x == my_x) then
      if (player_y == my_y) then
         API_SendMessage(myid, player, msg);
         --API_AddTimer(myid, player, dir);
      end
   end
end

function event_npc_move(player, max_count, dir, msg)
    if(count >= max_count - 1) then
        API_SendMessage(myid, player, msg);
        count = 0;
    else
        API_AddTimer(myid, player, dir);
        count = count + 1;
    end
end
